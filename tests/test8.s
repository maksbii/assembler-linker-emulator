.section my_code
 st %r1, [%r2]
 st %r1, [%r2+10]
 st %r1, 20
 st %r1, 5000
 st %r1, bar
 halt
bar: .word 0
.end
