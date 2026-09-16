#ifndef BIGMATH_H
#define BIGMATH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/*
  Представление больших целых чисел.
  - digits: указатель на массив слов (либо uint32_t, либо uint64_t в зависимости от WORD_BITS)
  - length: число слов в массиве (используется фактическое количество, leading zeros допустимы)
  - sign: 0 для неотрицательных (здесь обычно только неотрицательные числа для мод.арифметики)
  Реализация деталей (выделение памяти, normalize, переносы) скрываются в .c.
*/
typedef struct {
    uint32_t *digits;   /* или uint64_t * при выборе 64-битного слова */
    size_t length;      /* длина массива digits (кол-во слов) */
    int sign;           /* 1 или 0 — для общей полноты; для мод.арифм. обычно 0 */
} BigInt;

/*
  Контекст Монтгомери — предвычисленные параметры для модуля N:
  R = base^k (обычно base = 2^32 или 2^64), R_inv, N', и размер в словах.
  Все поля управляются реализацией; структура — opaque для пользователя,
  но в C мы даём тип (реализацию можно скрыть в .c через incomplete type при желании).
*/
typedef struct {
    BigInt *mod;        /* N */
    BigInt *R;          /* R (в representation) */
    BigInt *R_inv;      /* R^{-1} mod N */
    uint32_t nprime;    /* -N^{-1} mod base (single-word) — используется в алгоритме */
    size_t n_words;     /* число слов модуля */
    /* дополнительные поля (внутренние буферы и т.п.) могут быть добавлены в реализации */
} MontgomeryCtx;


/--- Создание / освобождение BigInt ---/

/* Создать пустой (нулевой) BigInt с capacity = words */
BigInt *bigint_new(size_t words);

/* Создать BigInt из строки шестнадцатеричного представления ("A1B2...") */
/* Возвращает NULL при ошибке */
BigInt *bigint_from_hex(const char *hexstr);

/* Создать BigInt из десятичной строки */
BigInt *bigint_from_dec(const char *decstr);

/* Копирование */
BigInt *bigint_copy(const BigInt *a);

/* Освободить BigInt */
void bigint_free(BigInt *a);

/* Преобразовать BigInt в NUL-terminated hex string (память выделяется, нужно free) */
char *bigint_to_hex(const BigInt *a);

/* Нормализация/сравнение (возвращает -1,0,1 как strcmp) */
int bigint_cmp(const BigInt *a, const BigInt *b);


/--- Базовые операции (на уровне интерфейса) ---/

/* Наивное умножение: r = a * b (возвращает новый BigInt) */
BigInt *bigint_mul_naive(const BigInt *a, const BigInt *b);

/* Наивное редуцирование: r = a mod m */
BigInt *bigint_mod_naive(const BigInt *a, const BigInt *m);

/* Быстрое возведение в степень (square-and-multiply), наивные умножения/редукции:
   r = base^exp mod mod; возвращает новый BigInt */
BigInt *mod_exp(const BigInt *base, const BigInt *exp, const BigInt *mod);


/--- Интерфейс Монтгомери ---/

/* Инициализация контекста Монтгомери для модуля N.
   Возвращает указатель на MontgomeryCtx (или NULL при ошибке).
   После использования — вызвать montgomery_free(). */
MontgomeryCtx *montgomery_init(const BigInt *mod);

/* Освободить контекст Монтгомери */
void montgomery_free(MontgomeryCtx *ctx);

/* Перевод a в Монтгомери-репрезентацию: a_bar = a * R mod N */
BigInt *montgomery_to_mont(const MontgomeryCtx *ctx, const BigInt *a);

/* Обратное преобразование: a = a_bar * R^{-1} mod N */
BigInt *montgomery_from_mont(const MontgomeryCtx *ctx, const BigInt *a_bar);

/* Монтгомери-умножение: r = a * b * R^{-1} mod N (внутри принимает числа в mont-репрезентации или обычной — см соглашение).
   Возвращает новый BigInt (в mont-репрезентации). */
BigInt *montgomery_mul(const MontgomeryCtx *ctx, const BigInt *a, const BigInt *b);

/* Возведение в степень с использованием Монтгомери (эффективно для больших exp):
   r = base^exp mod N.
   Возвращает новый BigInt (в обычной репрезентации). */
BigInt *montgomery_exp(const MontgomeryCtx *ctx, const BigInt *base, const BigInt *exp);


/--- Вспомогательные/диагностические функции (по желанию) ---/

/* Быстрая проверка равенства по модулю: returns true если (a mod m) == (b mod m) */
bool bigint_equal_mod(const BigInt *a, const BigInt *b, const BigInt *m);

/* Преобразование из/в uint64_t (если помещается) */
BigInt *bigint_from_u64(uint64_t v);
uint64_t bigint_to_u64_or_die(const BigInt *a); /* поведение при переполнении — документировать/обрабатывать */

#endif /* BIGMATH_H */
