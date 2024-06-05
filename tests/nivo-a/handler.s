.extern isr_timer, isr_terminal, isr_software

.global handler
.section my_handler
.equ line_feed, 0xA
.equ carriage_return, 0xD
.equ terminal_out, 0xFFFFFF00
.equ message_len, message_end - message_start
handler:
    push %r1
    push %r2
    csrrd %cause, %r1
    ld $2, %r2
    beq %r1, %r2, handle_timer
    ld $3, %r2
    beq %r1, %r2, handle_terminal
    ld $4, %r2
    beq %r1, %r2, handle_software
    ld $1, %r2
    beq %r1, %r2, handle_illegal_instruction   
finish:
    pop %r2
    pop %r1
    iret
# obrada prekida od tajmera
handle_timer:
    call isr_timer
    jmp finish
# obrada prekida od terminala
handle_terminal:
    call isr_terminal
    jmp finish
# obrada softverskog prekida
handle_software:
    call isr_software
    jmp finish
handle_illegal_instruction:
loop:
    ld $message_start, %r3
    add %r2, %r3
    ld [%r3], %r3
    st %r3, terminal_out
    ld $1, %r1
    add %r1, %r2
    ld $message_len, %r1
    bne %r1, %r2, loop
    ld $line_feed, %r1
    st %r1, terminal_out
    ld $carriage_return, %r1
    st %r1, terminal_out
    pop %r3
    pop %r2
    pop %r1
    halt
    ret
message_start:
.ascii "illegal instruction"
message_end:
.end
