

.section moj_kod

ld $a, %r1
ld $b, %r2
c:
.skip 8


.section moji_podaci

.word a
.equ a, c + 7
.equ b, a

.end

.section sekcija
.skip 8

