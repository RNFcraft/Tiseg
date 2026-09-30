# Tiseg — 7-сегментный дисплей, кнопка и простой таймер для Arduino

[![Platform](https://img.shields.io/badge/platform-Arduino-blue)](https://www.arduino.cc)
[![Dependencies](https://img.shields.io/badge/dependencies-none-green)](#установка)

Для всей библиотеки достаточно одного подключения:

```cpp
#include <Tiseg.h>
```

После этого доступны три уровня API:

- `Tiseg<DIGITS>` — только вывод чисел на дисплей;
- `TisegButton` — отдельная кнопка с антидребезгом;
- `TisegTimer<DIGITS>` — готовый таймер с одной кнопкой.

Все компоненты неблокирующие и работают через `tick()`.

## Быстрый старт: таймер 0 → 60 одной кнопкой

Для задачи с четырёхразрядным индикатором:

```cpp
#include <Tiseg.h>

const uint8_t digitPins[] = {13, 12, 11, 10};
const uint8_t segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 1};

TisegTimer<4> timer(digitPins, segmentPins, 9, 60);

void setup() {
    timer.begin();
}

void loop() {
    timer.tick();
}
```

Подключение сегментов:

```text
A  -> 2
B  -> 3
C  -> 4
D  -> 5
E  -> 6
F  -> 7
G  -> 8
DP -> 1

Разряды общего анода -> 13, 12, 11, 10
Кнопка -> 9 и GND
```

### Поведение таймера

После `timer.begin()` дисплей показывает:

```text
0000
```

Первое нажатие запускает таймер. Во время счёта ведущие нули не выводятся:

```text
___0
___1
___2
...
__12
...
__59
```

Второе нажатие ставит таймер на паузу и включает ведущие нули:

```text
0001
0012
0059
```

Третье нажатие сбрасывает таймер:

```text
0000
```

Если таймер сам доходит до максимума `60`, он останавливается и показывает:

```text
0060
```

Следующее нажатие после завершения сбрасывает его в `0000`.

## `TisegTimer`

Создание:

```cpp
TisegTimer<4> timer(digitPins, segmentPins, buttonPin, maxSeconds);
```

Например:

```cpp
TisegTimer<4> timer(digitPins, segmentPins, 9, 60);
```

Основные методы:

```cpp
timer.begin();
timer.tick();
timer.start();
timer.pause();
timer.reset();
```

Текущее значение:

```cpp
unsigned long value = timer.value();
```

Текущее состояние:

```cpp
TisegTimer<4>::State state = timer.state();
```

Для расширенных случаев можно получить внутренние объекты:

```cpp
timer.display();
timer.button();
```

## Обычный вывод чисел через `Tiseg`

```cpp
#include <Tiseg.h>

const uint8_t digitPins[] = {13, 12, 11, 10};
const uint8_t segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 1};

Tiseg<4> display(digitPins, segmentPins);

void setup() {
    display.begin();
    display.printR(1);
}

void loop() {
    display.tick();
}
```

Вывод справа:

```cpp
display.printR(1);        // ___1
display.printR(12);       // __12
display.printR(12, true); // 0012
```

Вывод слева:

```cpp
display.printL(1);        // 1___
display.printL(12);       // 12__
display.printL(12, true); // 1200
```

`print()` является сокращением для `printR()`:

```cpp
display.print(123);       // _123
display.print(123, true); // 0123
```

Очистка:

```cpp
display.clear();
```

## `TisegButton`

Отдельный include не нужен:

```cpp
#include <Tiseg.h>

TisegButton button(9);
```

В `setup()`:

```cpp
button.begin();
button.onPress(myFunction);
```

В `loop()`:

```cpp
button.tick();
```

Доступны:

```cpp
button.onPress(myFunction);
button.onRelease(myFunction);
button.isPressed();
button.wasPressed();
button.wasReleased();
```

По умолчанию используется `INPUT_PULLUP`, поэтому кнопка подключается между Arduino-пином и `GND`.

## Установка

Скопируйте папку `Tiseg` в `Documents/Arduino/libraries` или установите ZIP через Arduino IDE.

Внешних зависимостей нет.

## Структура

```text
Tiseg/
├── Tiseg.h
├── TisegButton.h
├── TisegTimer.h
├── library.properties
└── examples/
    ├── BasicDisplay/
    │   └── BasicDisplay.ino
    └── ButtonTimer/
        └── ButtonTimer.ino
```
