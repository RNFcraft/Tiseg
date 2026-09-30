# Tiseg — простой драйвер 7-сегментного дисплея для Arduino

[![Platform](https://img.shields.io/badge/platform-Arduino-blue)](https://www.arduino.cc)
[![Dependencies](https://img.shields.io/badge/dependencies-none-green)](#установка)

**Tiseg** — небольшая шаблонная библиотека для вывода целых чисел на многоразрядный 7-сегментный индикатор с общим анодом.

Основной класс `Tiseg` занимается только дисплеем: хранит изображение, преобразует число в сегменты и выполняет динамическую индикацию. Для проектов с кнопкой в библиотеке есть отдельный необязательный помощник `TisegButton`, который не смешивает логику кнопки с драйвером дисплея.

## Возможности

- `print(num)` — обычный вывод числа справа.
- `printR(num)` — выравнивание числа по правому краю.
- `printL(num)` — выравнивание числа по левому краю.
- Неиспользуемые разряды можно оставить пустыми или заполнить нулями.
- Поддерживаются отрицательные целые числа.
- Любое количество разрядов через шаблон: `Tiseg<2>`, `Tiseg<4>`, `Tiseg<6>` и т.д.
- Встроенное неблокирующее мультиплексирование.
- `TisegButton` — необязательная кнопка с антидребезгом и callback-функциями.
- Внешних зависимостей нет.

## Установка

Скопируйте папку `Tiseg` в `Documents/Arduino/libraries` либо установите библиотеку ZIP-архивом через Arduino IDE.

## Подключение дисплея

Конструктор принимает два массива:

```cpp
const uint8_t digitPins[]   = {13, 12, 11, 10};
const uint8_t segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 1};

Tiseg<4> display(digitPins, segmentPins);
```

- `digitPins` — пины разрядов, их количество должно совпадать с `Tiseg<DIGITS>`.
- `segmentPins` — восемь пинов сегментов в порядке `a, b, c, d, e, f, g, dp`.
- Текущая реализация рассчитана на индикатор с **общим анодом**.

## Быстрый старт

```cpp
#include <Tiseg.h>

const uint8_t digitPins[]   = {13, 12, 11, 10};
const uint8_t segmentPins[] = {2, 3, 4, 5, 6, 7, 8, 1};

Tiseg<4> display(digitPins, segmentPins);

void setup() {
    display.begin();
    display.printR(1);     // ___1
}

void loop() {
    display.tick();        // обязательно вызывать постоянно
}
```

## API дисплея

### `begin()`

Настраивает пины дисплея. Вызывается один раз в `setup()`.

### `tick()`

Обновляет один разряд динамической индикации. Метод нужно вызывать как можно чаще в `loop()`.

```cpp
void loop() {
    display.tick();
}
```

`tick()` не содержит `delay()` и не блокирует основную программу.

### `print(num, fillZeros = false)`

Эквивалент `printR()` — число выводится справа.

```cpp
display.print(1);        // ___1
display.print(1, true);  // 0001
```

### `printR(num, fillZeros = false)`

Выравнивает число по правому краю:

```cpp
display.printR(1);         // ___1
display.printR(12);        // __12
display.printR(12, true);  // 0012
display.printR(1234);      // 1234
```

### `printL(num, fillZeros = false)`

Выравнивает число по левому краю:

```cpp
display.printL(1);         // 1___
display.printL(12);        // 12__
display.printL(12, true);  // 1200
display.printL(1234);      // 1234
```

Здесь `_` означает погашенный разряд.

### `clear()`

Очищает буфер дисплея:

```cpp
display.clear();
```

## Дополнительный класс `TisegButton`

Кнопка подключается отдельным заголовком:

```cpp
#include <TisegButton.h>

TisegButton button(9);
```

По умолчанию используется `INPUT_PULLUP`, поэтому подключение простое:

```text
Arduino pin ---- button ---- GND
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

При каждом нормальном нажатии после антидребезга библиотека один раз вызовет назначенную функцию:

```cpp
void myFunction() {
    // действие при нажатии
}
```

Можно также назначить функцию на отпускание:

```cpp
button.onRelease(myReleaseFunction);
```

Или работать без callback-функций:

```cpp
if (button.wasPressed()) {
    // одно событие на одно нажатие
}
```

Текущее стабильное состояние доступно через:

```cpp
if (button.isPressed()) {
    // кнопка сейчас удерживается
}
```

### Настройка антидребезга

По умолчанию антидребезг равен 50 мс:

```cpp
TisegButton button(9);
```

Можно задать другое значение:

```cpp
TisegButton button(9, 80); // 80 мс
```

`TisegButton` не использует `delay()`, поэтому дисплей продолжает нормально мультиплексироваться во время работы кнопки.

## Таймер одной кнопкой: старт → пауза → сброс

Полный пример находится в `examples/ButtonTimer/ButtonTimer.ino`.

Главная идея — состояние таймера хранится в пользовательском коде:

```cpp
enum TimerState {
    TIMER_READY,
    TIMER_RUNNING,
    TIMER_PAUSED
};
```

Кнопке назначается одна функция:

```cpp
button.onPress(handleTimerButton);
```

А функция меняет действие в зависимости от текущего состояния:

```cpp
void handleTimerButton() {
    switch (timerState) {
        case TIMER_READY:
            // первое нажатие — старт
            timerState = TIMER_RUNNING;
            lastSecondAt = millis();
            break;

        case TIMER_RUNNING:
            // второе нажатие — пауза
            timerState = TIMER_PAUSED;
            break;

        case TIMER_PAUSED:
            // третье нажатие — сброс
            timerState = TIMER_READY;
            seconds = 0;
            display.printR(0, true);
            break;
    }
}
```

После сброса следующее нажатие снова запускает таймер.

Такой подход специально разделяет обязанности:

- `Tiseg` отвечает только за дисплей;
- `TisegButton` отвечает только за кнопку и антидребезг;
- логика таймера остаётся в скетче и легко меняется под конкретный проект.

## Пример изменения числа

`print()` только меняет содержимое буфера, поэтому его можно вызывать тогда, когда значение действительно изменилось:

```cpp
int value = 0;
unsigned long lastUpdate = 0;

void loop() {
    display.tick();

    if (millis() - lastUpdate >= 1000) {
        lastUpdate = millis();
        value++;
        display.printR(value, true);
    }
}
```

## Если число не помещается

Если число содержит больше цифр, чем доступно на дисплее, лишние старшие цифры отбрасываются.

Например, на четырёхразрядном дисплее:

```cpp
display.printR(12345); // 2345
```

Для отрицательного числа один разряд резервируется под знак минус.

## Структура репозитория

```text
Tiseg/
├── Tiseg.h
├── TisegButton.h
├── library.properties
└── examples/
    ├── BasicDisplay/
    │   └── BasicDisplay.ino
    └── ButtonTimer/
        └── ButtonTimer.ino
```
