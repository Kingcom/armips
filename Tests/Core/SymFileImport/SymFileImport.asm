.nds
.create "output.bin",0

.if defined(@@LocalLabel)
	.error "@@LocalLabel should not be accessible here"
.endif
.if defined(@StaticLabel)
	.error "@StaticLabel should not be accessible here"
.endif

.importsym "input.sym"

.if defined(@@LocalLabel)
	.error "@@LocalLabel should not be accessible here"
.endif
.if defined(@StaticLabel)
	.error "@StaticLabel should not be accessible here"
.endif

.dd	ValidLabel
.dd	ValidLabelWithSpaces
.dd	ValidLabelWithTabs
.dd	ValidLabelWithSpacesAndTabs
.dd	ValidLabelWithComment

ldr	r0,=0xABCDABCD
.pool

.org 0x12345678
NewlyAddedLabel1:

.definelabel NewlyAddedLabel2, 0x87654321

.skip 4
@NewlyAddedStaticLabel:

.skip 4
@@NewlyAddedLocalLabel:

.close
