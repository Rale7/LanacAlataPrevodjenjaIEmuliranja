# file: main.s
.extern handler
.global my_start, my_counter, cr

.section code
.equ initial_sp, 0xFFFFFEFE
.equ timer_config, 0xFFFFFF10
.equ cr, 0xD
my_start:
    ld $initial_sp, %sp
    ld $handler, %r1
    csrwr %r1, %handler

    ld $0x1, %r1
    st %r1, timer_config
wait:
    ld my_counter, %r1
    ld $20, %r2
    bne %r1, %r2, wait
    halt

.section my_data
my_counter:
.word 0

.end
