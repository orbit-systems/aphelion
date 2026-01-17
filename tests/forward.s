

.section text

.align 4
mystring_len:
    .dl (mystring_end - mystring)

mystring:
    .ds "hello!"
mystring_end:
