#pragma once
#include <Arduino.h>

/**
 * Класс Tiseg для управления таймером на семисегментном дисплее.
 * Шаблонный параметр DIGITS задает количество разрядов (например, 4).
 *
 * Библиотека самодостаточна: динамическая индикация реализована внутри
 * (методы multiplex/clearPrintR/clear/update), внешних зависимостей нет —
 * только Arduino API.
 */
template <uint8_t DIGITS>
class Tiseg {
  public:
    /**
     * Конструктор класса
     * @param digitPins   Массив пинов разрядов дисплея (DIGITS шт.)
     * @param segmentPins Массив пинов сегментов дисплея (8 шт.: a,b,c,d,e,f,g,dp)
     * @param buttonPin   Пин подключения кнопки (INPUT_PULLUP, нажатие = LOW)
     */
    Tiseg(const uint8_t* digitPins, const uint8_t* segmentPins, uint8_t buttonPin) :
        _digitPins(digitPins), _segmentPins(segmentPins), _btn(buttonPin) {}

    /**
     * Инициализация периферии. Вызывать внутри setup().
     */
    void begin() {
        pinMode(_btn, INPUT_PULLUP);
        for (uint8_t i = 0; i < DIGITS; i++) pinMode(_digitPins[i], OUTPUT);
        for (uint8_t i = 0; i < 8; i++) pinMode(_segmentPins[i], OUTPUT);
        showZero();
        // Стартовая задержка, как в исходном коде (неблокирующая: крутим tick())
        unsigned long start = millis();
        while (millis() - start < 2000) tick();
    }

    /**
     * Обновление динамической индикации дисплея.
     * Необходимо постоянно вызывать внутри главного loop().
     */
    void tick() {
        multiplex();
    }

    /**
     * Проверка нажатия кнопки (LOW-уровень) с защитой от дребезга.
     */
    bool isButtonPressed() {
        if (digitalRead(_btn) == LOW) {
            delay(50); // Базовый антидребезг
            if (digitalRead(_btn) == LOW) {
                return true;
            }
        }
        return false;
    }

    /**
     * Вывод нуля (соответствующего количества нулей) на дисплей.
     */
    void showZero() {
        clearPrintR(0);
        update();
    }

    /**
     * Запуск цикла таймера от 0 до указанного числа секунд.
     * @param maxSeconds Время, до которого идет отсчет (например, 60).
     */
    void runTimer(int maxSeconds) {
        for (int num = 0; num <= maxSeconds; num++) {
            clearPrintR(num);
            update();

            bool blinked = false; // Флаг: моргали ли уже в этой секунде
            unsigned long startTime = millis();

            // Внутренний цикл удержания одной секунды
            while (millis() - startTime < 1000) {
                tick(); // Постоянное обновление дисплея во время ожидания
                unsigned long elapsed = millis() - startTime;

                // Спустя 500 мс тушим экран для эффекта мигания
                if (elapsed >= 500 && !blinked) {
                    clear();
                    update();
                    blinked = true;
                }

                // Проверка нажатия кнопки во второй половине секунды (пауза/выход)
                if (elapsed > 500 && digitalRead(_btn) == LOW) {
                    if (handlePauseAndExit(num, blinked)) {
                        return; // Прерываем таймер и возвращаемся в loop
                    }
                }
            }

            // Если фаза мигания активна, возвращаем число перед новой секундой
            if (blinked) {
                clearPrintR(num);
                update();
            }
        }

        // Таймер дошел до конца, переходим в режим ожидания сброса
        waitForReset();
    }

  private:
    const uint8_t* _digitPins;   // Пины разрядов (общий анод)
    const uint8_t* _segmentPins; // Пины сегментов
    uint8_t _btn;                // Номер пина кнопки

    uint8_t _screen[DIGITS] = {0};  // Текущее изображение на дисплее
    uint8_t _digit = 0;             // Активный разряд при мультиплексировании
    unsigned long _stepAt = 0;      // Millis последнего переключения разряда

    // Коды сегментов для цифр 0-9: биты a,b,c,d,e,f,g,dp (бит 7 = dp)
    static const uint8_t SEG7[10];

    /**
     * Светит один разряд согласно _screen[] (общий анод: разряд LOW, сегменты LOW = горят).
     */
    void multiplex() {
        unsigned long now = millis();
        if (now - _stepAt < 2) return; // ~разрешение мультиплексирования
        _stepAt = now;

        // Погасить все разряды
        for (uint8_t i = 0; i < DIGITS; i++) digitalWrite(_digitPins[i], HIGH);

        // Подготовить сегменты текущего разряда
        uint8_t code = _screen[_digit];
        for (uint8_t s = 0; s < 8; s++) {
            digitalWrite(_segmentPins[s], (code & (1 << s)) ? LOW : HIGH);
        }

        // Включить текущий разряд
        digitalWrite(_digitPins[_digit], LOW);

        if (++_digit >= DIGITS) _digit = 0;
    }

    /**
     * Очистить изображение и вывести число справа налево (аналог clearPrintR).
     */
    void clearPrintR(int num) {
        clear();
        bool neg = (num < 0);
        unsigned long v = neg ? (unsigned long)(-(num + 1)) + 1UL : (unsigned long)num;
        int8_t pos = (int8_t)DIGITS - 1;
        do {
            uint8_t d = v % 10;
            v /= 10;
            _screen[pos] = SEG7[d];
            if (pos == (int8_t)DIGITS - 2) _screen[pos] |= 0x80; // десятичная точка после целых
            pos--;
        } while (v > 0 && pos >= 0);
        if (neg && pos >= 0) _screen[pos] = 0x40; // минус (сегмент g)
    }

    /**
     * Погасить всё изображение (аналог clear()).
     */
    void clear() {
        for (uint8_t i = 0; i < DIGITS; i++) _screen[i] = 0;
    }

    /**
     * Немедленно применить текущее изображение (аналог update()).
     */
    void update() {
        _digit = 0;
        _stepAt = 0;
        multiplex();
    }

    /**
     * Внутренний метод обработки паузы и аварийного выхода.
     * Возвращает true, если нужно полностью прервать выполнение таймера.
     */
    bool handlePauseAndExit(int currentNum, bool& blinked) {
        // Ожидаем отпускания кнопки (встали на паузу)
        while (digitalRead(_btn) == LOW) {
            tick();
        }

        // Во время паузы возвращаем текущее число на экран, чтобы оно горело стабильно
        if (blinked) {
            clearPrintR(currentNum);
            update();
        }

        // Ждем следующего нажатия кнопки (сигнал к сбросу)
        while (digitalRead(_btn) == HIGH) {
            tick();
        }
        // Ждем отпускания после нажатия на сброс
        while (digitalRead(_btn) == LOW) {
            tick();
        }

        // Анимация возврата к исходному состоянию
        showZero();
        delayMillis(200);
        showZero();
        delayMillis(500);
        return true;
    }

    /**
     * Ожидание нажатия кнопки для сброса после того, как таймер успешно завершился.
     */
    void waitForReset() {
        while (digitalRead(_btn) == HIGH) {
            tick();
        }
        while (digitalRead(_btn) == LOW) {
            tick();
        }
        showZero();
        delayMillis(200);
        showZero();
        delayMillis(500);
    }

    /**
     * Неблокирующая задержка: ожидание ms миллисекунд с продолжением работы дисплея.
     */
    void delayMillis(unsigned long ms) {
        unsigned long start = millis();
        while (millis() - start < ms) tick();
    }
};

// Таблица кодов сегментов: a=0 b=1 c=2 d=3 e=4 f=5 g=6 dp=7
template <uint8_t DIGITS>
const uint8_t Tiseg<DIGITS>::SEG7[10] = {
    0x3F, // 0: a b c d e f
    0x06, // 1: b c
    0x5B, // 2: a b d e g
    0x4F, // 3: a b c d g
    0x66, // 4: b c f g
    0x6D, // 5: a c d f g
    0x7D, // 6: a c d e f g
    0x07, // 7: a b c
    0x7F, // 8: все
    0x6F  // 9: a b c d f g
};
