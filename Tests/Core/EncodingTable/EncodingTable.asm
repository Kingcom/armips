.gba
.create "output.bin", 0

.loadtable "encoding.tbl"

.str "Hello World"
.align 16

.str "hello world"
.align 16

.str "Hello Wörld"
.align 16

.str "hello wörld"
.align 16

.close
