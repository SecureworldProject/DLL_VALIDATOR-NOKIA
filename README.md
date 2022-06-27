Programa que importa una DLL y comprueba si cumple los requisitos de Securemirror.
Ayuda a verificar si el cifrador o challenge creado se ejecuta de la manera deseada.

Debe tener la librería challenge_loader_python.dll accesible (en el directorio x64/release o donde se encuentre el ejecutable DLL_validator.exe).

Debe tener la librería python3.dll accesible. Se sobreentiende que debe tener instalado python 3.10. 
Las posibles rutas de instalación de python son:
  - C:\Program Files\Python310
  - C:\Users\username\AppData\Local\Programs\Python\Python310
  
Si tu challenge python usa un módulo instalable, debes copiar la librería python3.dll despues de instalar dicho módulo, ya que por cada instalación la librería python3.dll cambia.

El módulo python (fichero .py) que contiene tu challenge debe estar en el directorio donde se encuentre el mismo ejecutable (DLL_validator.exe) para que el programa validador pueda encontrarlo.

Si la ejecución desde visual studio te da algún problema, ejecuta todo desde línea de comando (no desde visual studio con proyecto DLL validator, pues hay cuestiones que afectan a la ejecución de hilos dentro del visual, y puedes sufrir una parada de ejecución cuando un hilo despierta).
