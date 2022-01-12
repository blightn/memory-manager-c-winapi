# Memory manager

Реализация менеджера памяти для Windows поверх системной кучи со специальным режимом отладки для обнаружения и устранения проблем при работе с памятью (выход за пределы буфера, утечка памяти и тд.). Написано на C с использованием MSVS 2019 и WinAPI с поддержкой потокобезопасности (критические секции).

Репозиторий содержит:
- решение MSVS 2019 (MemoryManager.sln);
- исходники (MemoryManager\);
- пример (Example\);
- тесты (Tests\);
- скрипты для очистки (clean.bat удаляет увесистую папку ipch в .vs\MemoryManager\v16\ и бинарники, clean_all.bat удаляет то же + всю папку .vs).

Решение состоит из двух проектов (Example и Tests) с динамической/статической конфигурациями для x86 и x64. Протестировано на Windows Vista и выше.
____
Memory manager implementation for Windows on top of the system heap with a special debug mode for troubleshooting memory usage issues (buffer overflows, memory leak, etc.). Written in C using MSVS 2019 and WinAPI with thread safety support (critical sections).

The repository contains:
- MSVS 2019 solution (MemoryManager.sln);
- source code (MemoryManager\);
- example (Example\);
- tests (Tests\);
- scripts for clean up (clean.bat removes the weighty ipch folder in .vs\MemoryManager\v16\ and also removes binaries, clean_all.bat removes the same as the previous one plus the entire .vs folder).

The solution consists of two projects (Example and Tests) with dynamic/static configurations for both x86 and x64 architectures. Tested on Windows Vista and above.

## Building

Microsoft Visual Studio 2019 is used to build the solution.

Once built, you can find the binaries in the Bins\ folder:

- x64\
  - Debug\
  - Debug (static)\
  - Release\
  - Release (static)\
- x86\
	- Debug\
	- Debug (static)\
	- Release\
	- Release (static)\

## Authors

```
blightn <blightan@gmail.com>
```

## License

This project is licensed under the MIT License. See LICENSE in the project's root directory.
