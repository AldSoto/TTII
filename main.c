#include "mongoose.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef enum {q0, q1, q2, q3} ESTADOS;
typedef enum {S, B, P, A, I, F} COMANDOS;
char *comandos[] = {"SUBIR", "BAJAR", "PARAR", "ABRIR/CERRAR", "INICIO", "FIN"};

// Declaración de `opts` globalmente
static struct mg_http_serve_opts opts = {
    .root_dir = ".",  // Directorio raíz para servir archivos
};

// Función para verificar si un texto es solo espacios o está vacío
bool is_empty(const char *str) {
    while (*str) {
        if (!isspace(*str)) {
            return false; // No está vacío, contiene al menos un carácter que no es espacio
        }
        str++;
    }
    return true; // Está vacío o solo contiene espacios
}

bool contains_command(char *line, char command) {
    // Remueve espacios antes de verificar el comando
    while (*line == ' ') {
        line++;
    }
    while (line[strlen(line) - 1] == ' ') {
        line[strlen(line) - 1] = '\0';
    }

    int length = strlen(line);
    for (int i = 0; i < length; i++) {
        if (line[i] == command || line[i] == (command - 'A' + 'a')) {
            return true;
        }
    }
    return false;
}


void process_command(char *cmd, int *current_piso, int total_pisos) {
    char token;
    int estado = q0;
    int comando = -1;
    int valor = -1;

    for (int i = 0; cmd[i]; i++) {
        token = cmd[i];

        switch (estado) {
            case q0:
                if (token == 'F' || token == 'f') {
                    comando = F;
                    estado = q3; // Comando "FIN"
                } else if (token == 'I' || token == 'i') {
                    comando = I;
                    estado = q3; // Cambia al estado final
                } else if (token == 'S' || token == 's') {
                    comando = S;
                    estado = q1;
                } else if (token == 'B' || token == 'b') {
                    comando = B;
                    estado = q1;
                } else if (token == 'P' || token == 'p') {
                    comando = P;
                    estado = q2; // Para dígitos entre 1 y 10
                } else if (token == 'A' || token == 'a') {
                    comando = A;
                    estado = q2; // Para dígitos entre 1 y 10
                } else if (token == ' ') {
                    continue; // Ignora espacios
                } else {
                    printf("Error en la sintaxis: El símbolo '%c' no pertenece al lenguaje\n", token);
                    return;
                }
                break;

            // Comandos "S" y "B" necesitan valores del 1 al 7
            case q1:
                if (token == ' ') {
                    continue; // Ignora espacios
                } else if (token >= '1' && token <= '7') {
                    valor = token - '0';
                    estado = q3; // Cambia al estado final
                } else {
                    printf("Error en la sintaxis: El símbolo '%c' no pertenece al lenguaje\n", token);
                    return;
                }
                break;

            // Comandos "P" y "A" necesitan valores del 1 al 9
            case q2:
                if (token == ' ') {
                    continue;
                } else if ((token >= '1' && token <= '9') || (token == '0' && valor == 1)) {
                    if (token == '0' && valor == 1) {
                        valor = 10; // Valor 10 para "P" y "A"
                    } else {
                        valor = token - '0';
                    }
                    estado = q3; // Cambia al estado final
                } else {
                    printf("Error en la sintaxis: El símbolo '%c' no pertenece al lenguaje\n", token);
                    return;
                }
                break;

            case q3: // Estado final para todos los comandos
                if (token == ' ') {
                    continue; // Ignora espacios adicionales
                } else {
                    printf("Error en la sintaxis: El símbolo '%c' no pertenece al lenguaje\n", token);
                    return;
                }
                break;
        }
    }

    // Resultado de la ejecución de comandos
    if (comando == I) {
        printf("Comando: %s\n", comandos[comando]);
    } else if (comando == S) {
        if ((*current_piso + valor) <= total_pisos) {
            *current_piso += valor;
            printf("Comando: %s, Dígito: %d. Ahora en piso %d\n", comandos[comando], valor, *current_piso);
        } else {
            printf("Error: No se puede subir al piso %d. Excede el total de pisos (%d).\n", *current_piso + valor, total_pisos);
        }
    } else if (comando == B) {
        if ((*current_piso - valor) >= 1) {
            *current_piso -= valor;
            printf("Comando: %s, Dígito: %d. Ahora en piso %d\n", comandos[comando], valor, *current_piso);
        } else {
            printf("Error: No se puede bajar al piso %d. Está por debajo del primer piso.\n", *current_piso - valor);
        }
    } else if (comando == P || comando == A) {
        printf("Comando: %s, tiempo de espera: %d segundos\n", comandos[comando], valor);
    } else if(comando == F) {
        printf("Comando: FIN\n"); // Verifica el comando "FIN"

    }
}

char* validate_syntax(const char *text) {
    static char result[1024]; // Almacena el resultado de la validación
    const int total_pisos = 8;
    int current_piso = 1;

     if (is_empty(text)) {
        strcpy(result, "Error: Debe ingresar comandos válidos.\n"); // Mensaje de error para entrada vacía
        return result;
    }

    char text_copy[512]; // Copia para procesar
    strncpy(text_copy, text, sizeof(text_copy));
    text_copy[sizeof(text_copy) - 1] = '\0'; // Asegúrate de que esté terminado en nulo
    
    char *line = strtok(text_copy, "\n"); // Divide el texto en líneas
    bool found_inicial = false;
    bool found_final = false;
    bool intermediate_command = false;

    // Limpia el resultado antes de comenzar
    memset(result, 0, sizeof(result));

    int count_inicial = 0; // Para contar los "INICIO"
    int count_final = 0;   // Para contar los "FIN"

    while (line) {
        // Elimina espacios al principio y al final de cada línea
        while (*line == ' ') line++; // Espacios iniciales
        char *end = line + strlen(line) - 1;
        while (end > line && *end == ' ') {
            *end = '\0'; // Elimina espacios finales
            end--;
        }

        // Verifica el comando "I"
        if (contains_command(line, 'I')) {
            count_inicial++; // Cuenta los "INICIO"
            if (count_inicial > 1) {
                strcat(result, "Error: No puede haber más de un 'INICIO'.\n");
            } else {
                found_inicial = true;
            }
        } else if (!found_inicial) {
            strcat(result, "Error: Debe comenzar con 'INICIO'.\n");
        }

        // Verifica el comando "F"
        if (contains_command(line, 'F')) {
            count_final++; // Cuenta los "FIN"
            if (!found_inicial) {
                strcat(result, "Error: 'FIN' no puede existir sin 'INICIO'.\n");
            } else if (count_final > 1) {
                strcat(result, "Error: No puede haber más de un 'FIN'.\n");
            } else {
                found_final = true;
            }
        }

        // Verifica la presencia de al menos un comando entre "INICIO" y "FIN"
        if (found_inicial && !found_final) {
            if (contains_command(line, 'S') || contains_command(line, 'B') || contains_command(line, 'P') || contains_command(line, 'A')) {
                intermediate_command = true; // Comando válido entre "INICIO" y "FIN"
            }
        }

        // Procesa el comando y valida su contenido
        process_command(line, &current_piso, total_pisos);

        line = strtok(NULL, "\n"); // Siguiente línea
    }

    // Verificaciones finales después de procesar todas las líneas
    if (!found_inicial) {
        strcat(result, "Error: Se requiere 'INICIO'.\n");
    }

    if (!found_final) {
        strcat(result, "Error: Se requiere 'FIN'.\n");
    }

    if (!intermediate_command) {
        strcat(result, "Error: Se requiere al menos un comando entre 'INICIO' y 'FIN'.\n");
    }

    // Si no hay errores, considera la validación exitosa
    if (strlen(result) == 0) {
        strcat(result, "Comandos válidos\n");
    }

    return result; // Devuelve el resultado final
}

void ev_handler(struct mg_connection *c, int ev, void *ev_data) {
    if (ev == MG_EV_HTTP_MSG) {
        struct mg_http_message *hm = (struct mg_http_message *) ev_data;

           if (mg_http_match_uri(hm, "/")) {
            mg_http_serve_file(c, hm, "index.html", &opts); // Sirve el archivo HTML
        } else if (mg_http_match_uri(hm, "/styles.css")) {
            mg_http_serve_file(c, hm, "styles.css", &opts); // Sirve el archivo CSS
        } else if (mg_http_match_uri(hm, "/script.js")) {
            mg_http_serve_file(c, hm, "script.js", &opts); // Sirve el archivo JavaScript
        
        }else if (mg_http_match_uri(hm, "/validate")) {
            char text[512];
            mg_http_get_var(&hm->body, "text", text, sizeof(text));

            char* validation_result = validate_syntax(text);

            // Si la validación es exitosa, devolver respuesta 200 con el resultado
            if (strstr(validation_result, "Error") == NULL) {
                mg_http_reply(c, 200, "Content-Type: text/plain\r\n", validation_result);
            } else {
                mg_http_reply(c, 400, "Content-Type: text/plain\r\n", validation_result); // Responde con error
            }
        } else if (mg_http_match_uri(hm, "/download")) {
            mg_http_reply(c, 200, "Content-Disposition: attachment; filename=output.txt\r\n",
                "Aquí está tu archivo de texto");
        } else if (mg_http_match_uri(hm, "/upload")) {
            mg_http_reply(c, 200, "Content-Type: text/plain\r\n", "Cargar archivo");
        } else {
            mg_http_serve_file(c, hm, "index.html", &opts);
        }
    }
}

int main(void) {
    struct mg_mgr mgr;
    struct mg_connection *c;

    mg_mgr_init(&mgr); // Inicializa el manejador de conexiones
    c = mg_http_listen(&mgr, "http://localhost:8000", ev_handler, NULL); // Inicia el servidor HTTP

    if (c == NULL) {
        printf("Error al iniciar el servidor\n");
        return 1;
    }

    printf("Servidor HTTP iniciado en http://localhost:8000\n");

    while (true) { // Bucle para mantener el servidor ejecutándose
        mg_mgr_poll(&mgr, 1000); // Procesa eventos cada segundo
    }

    mg_mgr_free(&mgr); // Limpia el manejador de conexiones
    return 0;
}