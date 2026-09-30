# Tiseg — простой драйвер 7-сегментного дисплея для Arduino

[![Platform](https://img.shields.io/badge/platform-Arduino-blue)](https://www.arduino.cc)
[![Dependencies](https://img.shields.io/badge/dependencies-none-green)](#установка)

**Tiseg** — небольшая библиотека для вывода целых чисел на многоразрядный 7-сегментный индикатор с общим анодом.

Для использования дисплея и кнопки достаточно одного подключения:

```cpp
#include <Tiseg.h>
```

После этого доступны оба класса:

```cpp
Tiseg<4> display(digitPins, segmentPins);
TisegButton button(9);
```

`Tiseg` отвечает только за дисплей, а `TisegButton` — за кнопку и антидребезг. Они остаются независимыми, хотя подключаются одним заголовком.

## Возможности

- `print(num)` — вывод числа справа.
- `printR(num)` — выравнивание по правому краю.
- `printL(num)` — выравнивание по левому краю.
- Свободные разряды можно оставить пустыми или заполнить нулями.
- Поддерживаются отрицательные числа.
- Любое количество разрядов: `Tiseg<2>`, `Tiseg<4>`, `Tiseg<6>` и т.д.
- Неблокирующее мультиплексирование.
- `TisegButton` с антидребезгом, callback-функциями и event API.
- Внешних зависимостей нет.

## Установка

Скопируйте папку `Tiseg` в `Documents/Arduino/libraries` либо установите ZIP через Arduino IDE.

## Быстрый старт дисплея

```cpp
#include <Tiseg.h>

const uint8_t digitPins[]   = {13, 12, 11, 10};
const uint8_t segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 1};

Tiseg<4> display(digitPins, segmentPins);

void setup() {
    display.begin();
    display.printR(1); // ___1
}

void loop() {
    display.tick();
}
```

## API дисплея

### `begin()`

Настраивает пины дисплея. Вызывается один раз в `setup()`.

### `tick()`

Обновляет динамическую индикацию. Вызывать как можно чаще в `loop()`.

### `print(num, fillZeros = false)`

Эквивалент `printR()`:

```cpp
display.print(1);       // ___1
display.print(1, true); // 0001
```

### `printR(num, fillZeros = false)`

```cpp
display.printR(1);        // ___1
display.printR(12);       // __12
display.printR(12, true); // 0012
```

### `printL(num, fillZeros = false)`

```cpp
display.printL(1);        // 1___
display.printL(12);       // 12__
display.printL(12, true); // 1200
```

`_` означает погашенный разряд.

### `clear()`

```cpp
display.clear();
```

## Кнопка через тот же `Tiseg.h`

Отдельный `#include <TisegButton.h>` не нужен.

```cpp
#include <Tiseg.h>

TisegButton button(9);
```

По умолчанию используется `INPUT_PULLUP`:

```text
Arduino pin ---- button ---- GND
```

В `setup()` кнопку нужно активировать:

```cpp
button.begin();
button.onPress(myFunction);
```

`button.begin()` оставлен явно, потому что настройка Arduino-пина должна выполняться после запуска Arduino runtime, то есть из `setup()`.

В `loop()`:

```cpp
button.tick();
```

Callback на нажатие:

```cpp
void myFunction() {
    // действие
}
```

Также доступны:

```cpp
button.onRelease(myReleaseFunction);
button.isPressed();
button.wasPressed();
button.wasReleased();
```

Антидребезг по умолчанию — 50 мс:

```cpp
TisegButton button(9);
```

Можно изменить:

```cpp
TisegButton button(9, 80);
```

## Таймер одной кнопкой: старт → пауза → сброс

Полный пример: `examples/ButtonTimer/ButtonTimer.ino`.

Для него тоже нужен только один include:

```cpp
#include <Tiseg.h>

Tiseg<4> display(digitPins, segmentPins);
TisegButton button(9);
```

Кнопке назначается функция:

```cpp
button.onPress(handleTimerButton);
```

Состояния таймера:

```cpp
enum TimerState {
    TIMER_READY,
    TIMER_RUNNING,
    TIMER_PAUSED
};
```

Логика:

```cpp
void handleTimerButton() {
    switch (timerState) {
        case TIMER_READY:
            timerState = TIMER_RUNNING;
            lastSecondAt = millis();
            break;

        case TIMER_RUNNING:
            timerState = TIMER_PAUSED;
            break;

        case TIMER_PAUSED:
            timerState = TIMER_READY;
            seconds = 0;
            display.printR(0, true);
            break;
    }
}
```

Получается цикл:

```text
1-е нажатие -> старт
2-е нажатие -> пауза
3-е нажатие -> сброс в 0000
4-е нажатие -> новый старт
```

## Если число не помещается

Старшие лишние цифры отбрасываются:

```cpp
display.printR(12345); // 2345 на 4-разрядном дисплее
```

Для отрицательного числа один разряд резервируется под минус.

## Структура

```text
Tiseg/
├── Tiseg.h              # основной публичный include
├── TisegButton.h        # подключается автоматически из Tiseg.h
├── library.properties
└── examples/
    ├── BasicDisplay/
    │   └── BasicDisplay.ino
    └── ButtonTimer/
        └── ButtonTimer.ino
```
