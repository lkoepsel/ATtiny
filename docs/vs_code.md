# VS Code Configuration Files

As of Aug 18, 2026 for Raspberry Pi Trixie.

**Perform these steps in VS Code while connected remotely to the Raspberry Pi:**

## C/C++ Configuration Installation
**If you are on Windows, use Ctrl, instead of CMD.**

1. *CMD-Shift-P -> "C/C++" -> C/C++: Edit Configurations (JSON))*
2. Copy the content below then in *VS Code CMD-a* to select all of the existing *c_cpp_properties.json* file
3. *CMD-v* to paste and *CMD-s* to save. 

## c_cpp_properties.json
```json
{
    "configurations": [
        {
            "name": "AVR",
            "includePath": [
                "/usr/lib/gcc/avr/14.2.0/include/",
                "/usr/lib/gcc/avr/14.2.0/include-fixed/**",
                "/usr/lib/avr/include",
                "${workspaceFolder}/**"
            ],
            "defines": ["__AVR_ATtiny13a__"],
            "compilerPath": "/usr/bin/avr-gcc", 
            "compilerArgs": [ ],
            "cStandard": "c99",
            "cppStandard": "c++98",
            "intelliSenseMode": "${default}"
        }
    ],
    "version": 4
}
```

## extensions.json

```json
{
    "recommendations": [
    "rockcat.avr-support",
    "ms-vscode.cpptools"
  ]
}
```

## settings.json

```json
{
    "files.associations": {
        "*.make": "makefile",
        "*.S": "avr",
        "xarm.h": "c"
    },
    "workbench.colorCustomizations": {
      "terminal.background": "#1e1e1e",
      "terminal.foreground": "#cccccc",
      "terminal.ansiBlack": "#000000",
      "terminal.ansiRed": "#cd3131",
      "terminal.ansiGreen": "#0dbc79",
      "terminal.ansiYellow": "#e5e510",
      "terminal.ansiBlue": "#2472c8",
      "terminal.ansiMagenta": "#bc3fbc",
      "terminal.ansiCyan": "#11a8cd",
      "terminal.ansiWhite": "#e5e5e5"
    },
    "cSpell.words": [
      "Adafruit",
      "AREF",
      "atmega",
      "Atmel",
      "avrdude",
      "binutils",
      "cppcheck",
      "Datasheet",
      "functionname",
      "getchar",
      "invalidscanf",
      "Libc",
      "makefiles",
      "Mersenne",
      "microcontroller",
      "Microcontrollers",
      "millis",
      "minicom",
      "MPLAB",
      "objdump",
      "oneline",
      "Optiboot",
      "outerpins",
      "Pico",
      "pidev",
      "Pmem",
      "PULLUP",
      "pushbuttons",
      "recompiles",
      "scanf",
      "serialio",
      "studentn",
      "sysclock",
      "tinymt",
      "uart",
      "unolib",
      "usbserial",
      "wrapprint",
      "Wundef",
      "Xplained"
    ],
  "extensions.ignoreRecommendations": true,
    "remote.defaultExtensionsIfInstalledLocally": []
}
```

## Default Task Installation
**If you are on Windows, use Ctrl, instead of CMD.**

1. *CMD-Shift-P -> "task" -> Tasks: Configure Default Build Task _> Create...from template -> Others Example...* 
2. Copy the content below then in *VS Code CMD-A* to select all of the existing *tasks.json* file
3. *CMD-v* to paste and *CMD-s* to save. 

## tasks.json
```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "make",
            "detail": "Run make",
            "type": "shell",
            "command": "/usr/bin/make ${input:makeTarget}",
            "options": {
                "cwd": "${fileDirname}"
            },
            "presentation": {
                "reveal": "always",
                "panel": "dedicated"
            },
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "problemMatcher": []
        }
    ],
    "inputs": [
        {
            "type": "pickString",
            "id": "makeTarget",
            "description": "Select a make target",
            "options": [
                {   
                    "value": "compile",
                },
                {   
                    "value": "flash",
                },
                {   
                    "value": "clean",
                },
                {   
                    "value": "complete",
                },
                {   
                    "value": "verbose",
                },
                {   
                    "value": "env",
                },
                {   
                    "value": "help",
                }
            ],
            "default": " flash"
        }
    ]
}
```
