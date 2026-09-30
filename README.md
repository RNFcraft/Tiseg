# Tiseg — простой драйвер 7-сегментного дисплея для Arduino

[![Platform](https://img.shields.io/badge/platform-Arduino-blue)](https://www.arduino.cc)
[![Dependencies](https://img.shields.io/badge/dependencies-none-green)](#установка)

**Tiseg** — небольшая шаблонная библиотека для вывода целых чисел на мног разрядный 7-сегментный индикатор с общим анодом.

Библиотека занимается только дисплеем: хранит изображение, преобразует число в сегменты и выполняет динамическую индикацию. Таймеров, кнопок, пауз и другой прикладной логики внутри нет.

## Возможности

- `print(num)` — обычный вывод числа справа.
- `printR(num)` — выравнивание числа по правому краю.
- `printL(num)` — выравнивание числа по левому краю.
- Неиспользуемые разряды можно оставить пустыми или заполнить нулями.
- Поддерживаются отрицательные целые числа.
- Любое количество разрядов через шаблон: `Tiseg<2>`, `Tiseg<4>`, `Tiseg<6>` и т.д.
- Встроенное мультиплексирование, внешних зависимостей нет.

## Установка

Скопируйте папку `Tiseg` в `Documents/Arduino/libraries` либо установите библиотеку ZIP-архивом через Arduino IDE.

## Подключение

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

## API

### `begin()`

Настраивает пины дисплея. Вызывается один раз в `setup()`.

```cpp
display.begin();
```

### `tick()`

Обновляет один разряд динамической индикации. Метод нужно вызывать как можно чаще в `loop()`.

```cpp
void loop() {
    display.tick();
}
```

`tick()` не содержит `delay()` и не блокирует выполнение основной программы.

### `print(num, fillZeros = false)`

Эквивалент `printR()` — число выводится справа.

```cpp
display.print(1);        // ___1
display.print(1, true);  // 0001
```

### `printR(num, fillZeros = false)`

Выравнивает число по правому краю.

Для четырёхразрядного дисплея:

```cpp
display.printR(1);         // ___1
display.printR(12);        // __12
display.printR(12, true);  // 0012
display.printR(1234);      // 1234
```

### `printL(num, fillZeros = false)`

Выравнивает число по левому краю.

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

После этого `tick()` продолжает работать, но все разряды остаются погашенными.

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

При этом библиотека не управляет временем — `millis()` и логика счётчика находятся в пользовательском скетче.

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
├── library.properties
└── examples/
    └── BasicDisplay/
        └── BasicDisplay.ino
```
