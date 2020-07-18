.notice "sub/sub.asm"

.relativeinclude off

.include "a.asm"
.include "sub/a.asm"

.relativeinclude on
.include "../a.asm"
.include "a.asm"
