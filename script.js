// Función para validar el texto con el servidor HTTP
function validarTexto() {
    const textArea = document.getElementById('inputText');
    const resultadoArea = document.getElementById('resultArea'); // Área para mostrar resultados
    const text = textArea.value;

    fetch('/validate', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded',
        },
        body: `text=${encodeURIComponent(text)}`, // Codifica adecuadamente
    })
    .then((response) => response.text()) // Extrae el texto de la respuesta
    .then((validationResult) => {
        resultadoArea.textContent = validationResult; // Muestra el resultado de la validación
        if (validationResult.startsWith("Error")) {
            resultadoArea.style.color = "red"; // Texto rojo para errores
        } else {
            resultadoArea.style.color = "green"; // Texto verde para éxito
        }
    })
    .catch((error) => {
        console.error("Error durante la validación:", error);
        resultadoArea.textContent = "Error durante la validación.";
        resultadoArea.style.color = "red";
    });
}

// Otras funciones JavaScript para el manejo de archivos
function descargarArchivo() {
    fetch("/download")
    .then((response) => response.blob())
    .then((blob) => {
        const url = window.URL.createObjectURL(blob);
        const a = document.createElement("a");
        a.href = url;
        a.download = "output.txt";
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);
    });
}

function cargarArchivo() {
    const input = document.createElement("input");
    input.type = "file";
    input.accept = ".txt";
    input.addEventListener("change", function (event) {
        const file = event.target.files[0];
        const reader = new FileReader();
        reader.onload = function (e) {
            const textArea = document.getElementById("inputText");
            textArea.value = e.target.result; // Carga el contenido del archivo en el textarea
        };
        reader.readAsText(file);
    });
    input.click();
}

// Muestra un mensaje de ayuda
function mostrarAyuda() {
    alert("Página para validar comandos de ascensores. Puedes escribir comandos, validarlos, descargarlos, o cargarlos desde un archivo.");
}

// Limpia el texto del área de entrada
function borrarTexto() {
    const textArea = document.getElementById("inputText");
    textArea.value = ""; // Limpia el textarea
    const resultadoArea = document.getElementById("resultArea");
    resultadoArea.textContent = ""; // Limpia el área de resultados
}