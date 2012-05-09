// Включаем набор стандартных определений для микроконтроллера PIC32.
#include <p32xxxx.h>

        .text                       // Начинаем секцию выполняемого кода.
        .set noreorder              // Отключаем переупорядочивание инструкций

//
// Обработчик прерывания от таймера.
//
timer_handler:
	addiu	sp, sp, -136        // Выделяем в стеке 34 слова

	mfc0	k0, _CP0_EPC        // Сохраняем адрес возврата EPC
	sw	k0, 132(sp)

	mfc0	k1, _CP0_STATUS     // Сохраняем режим процессора Status
	sw	k1, 128(sp)

	mflo	k0
	sw	k0, 124(sp)         // Регистр LO

	mfhi	k1
	sw	k1, 120(sp)         // Регистр HI

	sw	ra, 116(sp)         // Регистр 31
	sw	s8, 112(sp)         // Регистр 30
	sw	t9, 108(sp)         // Регистр 25
	sw	t8, 104(sp)         // Регистр 24
	sw	s7, 100(sp)         // Регистр 23
	sw	s6, 96(sp)          // Регистр 22
	sw	s5, 92(sp)          // Регистр 21
	sw	s4, 88(sp)          // Регистр 20
	sw	s3, 84(sp)          // Регистр 19
	sw	s2, 80(sp)          // Регистр 18
	sw	s1, 76(sp)          // Регистр 17
	sw	s0, 72(sp)          // Регистр 16
	sw	t7, 68(sp)          // Регистр 15
	sw	t6, 64(sp)          // Регистр 14
	sw	t5, 60(sp)          // Регистр 13
	sw	t4, 56(sp)          // Регистр 12
	sw	t3, 52(sp)          // Регистр 11
	sw	t2, 48(sp)          // Регистр 10
	sw	t1, 44(sp)          // Регистр 9
	sw	t0, 40(sp)          // Регистр 8
	sw	a3, 36(sp)          // Регистр 7
	sw	a2, 32(sp)          // Регистр 6
	sw	a1, 28(sp)          // Регистр 5
	sw	a0, 24(sp)          // Регистр 4
	sw	v1, 20(sp)          // Регистр 3
	sw	v0, 16(sp)          // Регистр 2
	sw	$1, 12(sp)          // Регистр 1

        // Увеличиваем регистр Compare чтобы получить следующее
        // прерывание еще через миллисекунду.
	mfc0	v0, _CP0_COMPARE
	li	v1, 80000 / 2       // F_CPU / 1000 / 2
	addu	v0, v1
	mtc0	v0, _CP0_COMPARE

        // Сбрасываем флаг в контроллере прерываний.
        li      v0, 1 << _CORE_TIMER_IRQ
        la      t0, IFS0CLR
        sw      v0, (t0)            // IFS0CLR := 1 << _CORE_TIMER_IRQ

        // Наращиваем счётчик времени.
	lw	v1, %gp_rel(time_msec)(gp)
	addiu	v1, 1               // time_msec += 1
	sw	v1, %gp_rel(time_msec)(gp)

        // Запоминаем значение указателя стека для текущей задачи.
	move	a0, sp

	lw	v1, %gp_rel(current_task)(gp)
	li	v0, 1
	beq	v1, v0, task1
	li	v0, 2
	beq	v1, v0, task2
        nop
        b       notask
        nop

task1:  // Задача 1: переключаемся на задачу 2
	sw	a0, %gp_rel(task1_stack_pointer)(gp) // task1_stack_pointer = sp
	li	v1, 2
	sw	v1, %gp_rel(current_task)(gp)        // current_task = 2
	lw	sp, %gp_rel(task2_stack_pointer)(gp) // sp = task2_stack_pointer
	b       restore
	nop

task2:  // Задача 2: переключаемся на задачу 1
	sw	a0, %gp_rel(task2_stack_pointer)(gp) // task2_stack_pointer = sp
notask:	li	v1, 1
	sw	v1, %gp_rel(current_task)(gp)        // current_task = 1
	lw	sp, %gp_rel(task1_stack_pointer)(gp) // sp = task1_stack_pointer

restore:
        // Восстанавливаем все регистры из стека.
	lw	a1, 132(sp)
	mtc0	a1, _CP0_EPC

	lw	a1, 128(sp)
	mtc0	a1, _CP0_STATUS

	lw	a1, 124(sp)
	mtlo	a1

	lw	a1, 120(sp)
	mthi	a1

	lw	ra, 116(sp)
	lw	s8, 112(sp)
	lw	t9, 108(sp)
	lw	t8, 104(sp)
	lw	s7, 100(sp)
	lw	s6, 96(sp)
	lw	s5, 92(sp)
	lw	s4, 88(sp)
	lw	s3, 84(sp)
	lw	s2, 80(sp)
	lw	s1, 76(sp)
	lw	s0, 72(sp)
	lw	t7, 68(sp)
	lw	t6, 64(sp)
	lw	t5, 60(sp)
	lw	t4, 56(sp)
	lw	t3, 52(sp)
	lw	t2, 48(sp)
	lw	t1, 44(sp)
	lw	t0, 40(sp)
	lw	a3, 36(sp)
	lw	a2, 32(sp)
	lw	a1, 28(sp)
	lw	a0, 24(sp)
	lw	v1, 20(sp)
	lw	v0, 16(sp)
	lw	$1, 12(sp)

	addiu	sp, sp, 136         // Освобождаем место в стеке
	eret                        // Выходим из прерывания

        //
        // Специальная секция кода, которую загрузчик
        // разместит по адресу входа в прерывание 0.
        //
        .section .vector_0, "ax", @progbits
timer_vector:
        j       timer_handler
        nop
