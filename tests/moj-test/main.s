
.extern a
.section sekcija
ld $b, %r0
ld $c, %r0
.equ c, b + 1
.equ b, a + 3 * (5 + 2 * (3 - 1))
halt

.end
