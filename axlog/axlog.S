	.text
	.attribute	4, 16
	.attribute	5, "rv64i2p0_m2p0_a2p0_f2p0_d2p0_c2p0"
	.file	"axlog.c8b64888-cgu.0"
	.section	".text._ZN44_$LT$$RF$T$u20$as$u20$core..fmt..Display$GT$3fmt17h320847fd15db251aE","ax",@progbits
	.p2align	1
	.type	_ZN44_$LT$$RF$T$u20$as$u20$core..fmt..Display$GT$3fmt17h320847fd15db251aE,@function
_ZN44_$LT$$RF$T$u20$as$u20$core..fmt..Display$GT$3fmt17h320847fd15db251aE:
	ld	a2, 0(a0)
	ld	a3, 8(a0)
	mv	a4, a1
	mv	a0, a2
	mv	a1, a3
	mv	a2, a4
	tail	_ZN42_$LT$str$u20$as$u20$core..fmt..Display$GT$3fmt17h7dcb245b8fdf6354E
.Lfunc_end0:
	.size	_ZN44_$LT$$RF$T$u20$as$u20$core..fmt..Display$GT$3fmt17h320847fd15db251aE, .Lfunc_end0-_ZN44_$LT$$RF$T$u20$as$u20$core..fmt..Display$GT$3fmt17h320847fd15db251aE

	.section	".text._ZN44_$LT$$RF$T$u20$as$u20$core..fmt..Display$GT$3fmt17h9dbc321d37742d77E","ax",@progbits
	.p2align	1
	.type	_ZN44_$LT$$RF$T$u20$as$u20$core..fmt..Display$GT$3fmt17h9dbc321d37742d77E,@function
_ZN44_$LT$$RF$T$u20$as$u20$core..fmt..Display$GT$3fmt17h9dbc321d37742d77E:
	ld	a0, 0(a0)
	tail	_ZN59_$LT$core..fmt..Arguments$u20$as$u20$core..fmt..Display$GT$3fmt17h778581adceeeee7fE
.Lfunc_end1:
	.size	_ZN44_$LT$$RF$T$u20$as$u20$core..fmt..Display$GT$3fmt17h9dbc321d37742d77E, .Lfunc_end1-_ZN44_$LT$$RF$T$u20$as$u20$core..fmt..Display$GT$3fmt17h9dbc321d37742d77E

	.section	.text._ZN4core3fmt9Arguments6new_v117h392723960367d5ebE,"ax",@progbits
	.p2align	1
	.type	_ZN4core3fmt9Arguments6new_v117h392723960367d5ebE,@function
_ZN4core3fmt9Arguments6new_v117h392723960367d5ebE:
	addi	sp, sp, -64
	sd	ra, 56(sp)
	sltu	a6, a2, a4
	addi	a5, a4, 1
	sltu	a5, a5, a2
	or	a5, a6, a5
	bnez	a5, .LBB2_2
	sd	a1, 16(a0)
	sd	a2, 24(a0)
	sd	zero, 0(a0)
	sd	a3, 32(a0)
	sd	a4, 40(a0)
	ld	ra, 56(sp)
	addi	sp, sp, 64
	ret
.LBB2_2:
.LBB2_3:
	auipc	a1, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.2)
	addi	a1, a1, %pcrel_lo(.LBB2_3)
.LBB2_4:
	auipc	a3, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.3)
	addi	a3, a3, %pcrel_lo(.LBB2_4)
	addi	a0, sp, 8
	li	a2, 1
	li	a4, 0
	call	_ZN4core3fmt9Arguments6new_v117h392723960367d5ebE
.LBB2_5:
	auipc	a1, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.5)
	addi	a1, a1, %pcrel_lo(.LBB2_5)
	addi	a0, sp, 8
	call	_ZN4core9panicking9panic_fmt17h04306303e515d76aE
	unimp	
.Lfunc_end2:
	.size	_ZN4core3fmt9Arguments6new_v117h392723960367d5ebE, .Lfunc_end2-_ZN4core3fmt9Arguments6new_v117h392723960367d5ebE

	.section	".text._ZN4core3ptr34drop_in_place$LT$axlog..Logger$GT$17h828d12fb3f838e82E","ax",@progbits
	.p2align	1
	.type	_ZN4core3ptr34drop_in_place$LT$axlog..Logger$GT$17h828d12fb3f838e82E,@function
_ZN4core3ptr34drop_in_place$LT$axlog..Logger$GT$17h828d12fb3f838e82E:
	ret
.Lfunc_end3:
	.size	_ZN4core3ptr34drop_in_place$LT$axlog..Logger$GT$17h828d12fb3f838e82E, .Lfunc_end3-_ZN4core3ptr34drop_in_place$LT$axlog..Logger$GT$17h828d12fb3f838e82E

	.section	".text._ZN4core3ptr37drop_in_place$LT$core..fmt..Error$GT$17hd32a9656971dfae9E","ax",@progbits
	.p2align	1
	.type	_ZN4core3ptr37drop_in_place$LT$core..fmt..Error$GT$17hd32a9656971dfae9E,@function
_ZN4core3ptr37drop_in_place$LT$core..fmt..Error$GT$17hd32a9656971dfae9E:
	ret
.Lfunc_end4:
	.size	_ZN4core3ptr37drop_in_place$LT$core..fmt..Error$GT$17hd32a9656971dfae9E, .Lfunc_end4-_ZN4core3ptr37drop_in_place$LT$core..fmt..Error$GT$17hd32a9656971dfae9E

	.section	".text._ZN4core3ptr40drop_in_place$LT$log..SetLoggerError$GT$17hba5946db384333f1E","ax",@progbits
	.p2align	1
	.type	_ZN4core3ptr40drop_in_place$LT$log..SetLoggerError$GT$17hba5946db384333f1E,@function
_ZN4core3ptr40drop_in_place$LT$log..SetLoggerError$GT$17hba5946db384333f1E:
	ret
.Lfunc_end5:
	.size	_ZN4core3ptr40drop_in_place$LT$log..SetLoggerError$GT$17hba5946db384333f1E, .Lfunc_end5-_ZN4core3ptr40drop_in_place$LT$log..SetLoggerError$GT$17hba5946db384333f1E

	.section	".text._ZN4core3ptr46drop_in_place$LT$$RF$mut$u20$axlog..Logger$GT$17heb065581dc1c6a20E","ax",@progbits
	.p2align	1
	.type	_ZN4core3ptr46drop_in_place$LT$$RF$mut$u20$axlog..Logger$GT$17heb065581dc1c6a20E,@function
_ZN4core3ptr46drop_in_place$LT$$RF$mut$u20$axlog..Logger$GT$17heb065581dc1c6a20E:
	ret
.Lfunc_end6:
	.size	_ZN4core3ptr46drop_in_place$LT$$RF$mut$u20$axlog..Logger$GT$17heb065581dc1c6a20E, .Lfunc_end6-_ZN4core3ptr46drop_in_place$LT$$RF$mut$u20$axlog..Logger$GT$17heb065581dc1c6a20E

	.section	".text._ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$10write_char17h75f71f3ea8095724E","ax",@progbits
	.p2align	1
	.type	_ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$10write_char17h75f71f3ea8095724E,@function
_ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$10write_char17h75f71f3ea8095724E:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	sext.w	a0, a1
	li	a2, 128
	sw	zero, 4(sp)
	bgeu	a0, a2, .LBB7_2
	sb	a1, 4(sp)
	li	a1, 1
	j	.LBB7_7
.LBB7_2:
	srliw	a0, a1, 11
	bnez	a0, .LBB7_4
	srli	a0, a1, 6
	ori	a0, a0, 192
	sb	a0, 4(sp)
	andi	a0, a1, 63
	ori	a0, a0, 128
	sb	a0, 5(sp)
	li	a1, 2
	j	.LBB7_7
.LBB7_4:
	srliw	a0, a1, 16
	bnez	a0, .LBB7_6
	slli	a0, a1, 32
	srli	a0, a0, 32
	srliw	a2, a1, 12
	ori	a2, a2, 224
	sb	a2, 4(sp)
	slli	a0, a0, 52
	srli	a0, a0, 58
	ori	a0, a0, 128
	sb	a0, 5(sp)
	andi	a0, a1, 63
	ori	a0, a0, 128
	sb	a0, 6(sp)
	li	a1, 3
	j	.LBB7_7
.LBB7_6:
	slli	a0, a1, 32
	srli	a0, a0, 32
	slli	a2, a0, 43
	srli	a2, a2, 61
	ori	a2, a2, 240
	sb	a2, 4(sp)
	slli	a2, a0, 46
	srli	a2, a2, 58
	ori	a2, a2, 128
	sb	a2, 5(sp)
	slli	a0, a0, 52
	srli	a0, a0, 58
	ori	a0, a0, 128
	sb	a0, 6(sp)
	andi	a0, a1, 63
	ori	a0, a0, 128
	sb	a0, 7(sp)
	li	a1, 4
.LBB7_7:
	addi	a0, sp, 4
	call	__LogIf_console_write_str
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
.Lfunc_end7:
	.size	_ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$10write_char17h75f71f3ea8095724E, .Lfunc_end7-_ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$10write_char17h75f71f3ea8095724E

	.section	".text._ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$9write_fmt17h56a9bcec95552620E","ax",@progbits
	.p2align	1
	.type	_ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$9write_fmt17h56a9bcec95552620E,@function
_ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$9write_fmt17h56a9bcec95552620E:
	addi	sp, sp, -64
	sd	ra, 56(sp)
	ld	a0, 0(a0)
	ld	a2, 40(a1)
	ld	a3, 32(a1)
	sd	a0, 0(sp)
	sd	a2, 48(sp)
	sd	a3, 40(sp)
	ld	a0, 24(a1)
	ld	a2, 16(a1)
	ld	a3, 8(a1)
	ld	a1, 0(a1)
	sd	a0, 32(sp)
	sd	a2, 24(sp)
	sd	a3, 16(sp)
	sd	a1, 8(sp)
.LBB8_1:
	auipc	a1, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.0)
	addi	a1, a1, %pcrel_lo(.LBB8_1)
	mv	a0, sp
	addi	a2, sp, 8
	call	_ZN4core3fmt5write17hc94f11bd44ac9229E
	ld	ra, 56(sp)
	addi	sp, sp, 64
	ret
.Lfunc_end8:
	.size	_ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$9write_fmt17h56a9bcec95552620E, .Lfunc_end8-_ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$9write_fmt17h56a9bcec95552620E

	.section	".text._ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$9write_str17h984b477899bf930cE","ax",@progbits
	.p2align	1
	.type	_ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$9write_str17h984b477899bf930cE,@function
_ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$9write_str17h984b477899bf930cE:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	mv	a0, a1
	mv	a1, a2
	call	__LogIf_console_write_str
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
.Lfunc_end9:
	.size	_ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$9write_str17h984b477899bf930cE, .Lfunc_end9-_ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$9write_str17h984b477899bf930cE

	.section	".text._ZN50_$LT$axlog..Logger$u20$as$u20$core..fmt..Write$GT$9write_str17he0e0d822714c986cE","ax",@progbits
	.globl	_ZN50_$LT$axlog..Logger$u20$as$u20$core..fmt..Write$GT$9write_str17he0e0d822714c986cE
	.p2align	1
	.type	_ZN50_$LT$axlog..Logger$u20$as$u20$core..fmt..Write$GT$9write_str17he0e0d822714c986cE,@function
_ZN50_$LT$axlog..Logger$u20$as$u20$core..fmt..Write$GT$9write_str17he0e0d822714c986cE:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	mv	a0, a1
	mv	a1, a2
	call	__LogIf_console_write_str
	li	a0, 0
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
.Lfunc_end10:
	.size	_ZN50_$LT$axlog..Logger$u20$as$u20$core..fmt..Write$GT$9write_str17he0e0d822714c986cE, .Lfunc_end10-_ZN50_$LT$axlog..Logger$u20$as$u20$core..fmt..Write$GT$9write_str17he0e0d822714c986cE

	.section	".text._ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$7enabled17hb5d3b6f14a3b2756E","ax",@progbits
	.globl	_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$7enabled17hb5d3b6f14a3b2756E
	.p2align	1
	.type	_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$7enabled17hb5d3b6f14a3b2756E,@function
_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$7enabled17hb5d3b6f14a3b2756E:
	li	a0, 1
	ret
.Lfunc_end11:
	.size	_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$7enabled17hb5d3b6f14a3b2756E, .Lfunc_end11-_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$7enabled17hb5d3b6f14a3b2756E

	.section	".text._ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$3log17h95b606852221f78fE","ax",@progbits
	.globl	_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$3log17h95b606852221f78fE
	.p2align	1
	.type	_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$3log17h95b606852221f78fE,@function
_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$3log17h95b606852221f78fE:
	addi	sp, sp, -1120
	sd	ra, 1112(sp)
	sd	s0, 1104(sp)
	sd	s1, 1096(sp)
	sd	s2, 1088(sp)
	mv	s0, a1
	ld	a0, 72(a1)
	lw	a2, 48(a1)
	li	a1, 0
	sd	a0, 8(sp)
	beqz	a2, .LBB12_2
	lw	a1, 52(s0)
.LBB12_2:
	sw	a1, 20(sp)
	ld	a1, 56(s0)
	ld	a2, 64(s0)
	addi	a0, a0, -1
	slli	a0, a0, 3
.LBB12_10:
	auipc	a3, %pcrel_hi(.LJTI12_0)
	addi	a3, a3, %pcrel_lo(.LBB12_10)
	add	a0, a0, a3
	ld	a0, 0(a0)
	sd	a1, 24(sp)
	sd	a2, 32(sp)
	li	s2, 33
	li	s1, 93
	jr	a0
.LBB12_3:
	li	s2, 31
	li	s1, 91
	j	.LBB12_7
.LBB12_4:
	li	s2, 32
	li	s1, 92
	j	.LBB12_7
.LBB12_5:
	li	s2, 36
	li	s1, 96
	j	.LBB12_7
.LBB12_6:
	li	s1, 90
	li	s2, 90
.LBB12_7:
	call	__LogIf_current_time
	sd	a0, 200(sp)
	lui	a0, 67109
	addiw	a0, a0, -557
	slli	a0, a0, 32
	slli	a1, a1, 32
	mulhu	a0, a1, a0
	srli	a0, a0, 38
	sw	a0, 212(sp)
	sb	s1, 303(sp)
	addi	a0, sp, 8
	sd	a0, 352(sp)
.LBB12_11:
	auipc	a0, %pcrel_hi(_ZN49_$LT$log..Level$u20$as$u20$core..fmt..Display$GT$3fmt17h946e783953f086cbE)
	addi	a0, a0, %pcrel_lo(.LBB12_11)
	sd	a0, 360(sp)
	li	a0, 2
	sd	a0, 368(sp)
	sd	zero, 384(sp)
	li	a6, 5
	sd	a6, 392(sp)
	li	a7, 1
	slli	a2, a7, 37
	sd	a2, 400(sp)
	sb	zero, 408(sp)
	sd	zero, 416(sp)
.LBB12_12:
	auipc	s1, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.16)
	addi	s1, s1, %pcrel_lo(.LBB12_12)
	sd	s1, 320(sp)
	sd	a7, 328(sp)
	addi	a1, sp, 368
	sd	a1, 304(sp)
	sd	a7, 312(sp)
	addi	a1, sp, 352
	sd	a1, 336(sp)
	sd	a7, 344(sp)
	addi	a1, sp, 303
	sd	a1, 264(sp)
.LBB12_13:
	auipc	t2, %pcrel_hi(_ZN4core3fmt3num3imp51_$LT$impl$u20$core..fmt..Display$u20$for$u20$u8$GT$3fmt17h4c098a1e6cb48eb1E)
	addi	t2, t2, %pcrel_lo(.LBB12_13)
	sd	t2, 272(sp)
	addi	a1, sp, 304
	sd	a1, 280(sp)
.LBB12_14:
	auipc	a1, %pcrel_hi(_ZN59_$LT$core..fmt..Arguments$u20$as$u20$core..fmt..Display$GT$3fmt17h778581adceeeee7fE)
	addi	a1, a1, %pcrel_lo(.LBB12_14)
	sd	a1, 288(sp)
.LBB12_15:
	auipc	t1, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.9)
	addi	t1, t1, %pcrel_lo(.LBB12_15)
	sd	t1, 232(sp)
	li	a5, 3
	sd	a5, 240(sp)
	sd	zero, 216(sp)
	addi	a3, sp, 264
	sd	a3, 248(sp)
	sd	a0, 256(sp)
	addi	a3, sp, 24
	sd	a3, 552(sp)
.LBB12_16:
	auipc	a3, %pcrel_hi(_ZN44_$LT$$RF$T$u20$as$u20$core..fmt..Display$GT$3fmt17h320847fd15db251aE)
	addi	a3, a3, %pcrel_lo(.LBB12_16)
	sd	a3, 560(sp)
	addi	a3, sp, 20
	sd	a3, 568(sp)
.LBB12_17:
	auipc	a3, %pcrel_hi(_ZN4core3fmt3num3imp52_$LT$impl$u20$core..fmt..Display$u20$for$u20$u32$GT$3fmt17hd08ec8485c71ebb2E)
	addi	a3, a3, %pcrel_lo(.LBB12_17)
	sd	a3, 576(sp)
.LBB12_18:
	auipc	a4, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.19)
	addi	a4, a4, %pcrel_lo(.LBB12_18)
	sd	a4, 520(sp)
	sd	a5, 528(sp)
	sd	zero, 504(sp)
	addi	a4, sp, 552
	sd	a4, 536(sp)
	sd	a0, 544(sp)
.LBB12_19:
	auipc	t0, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.10)
	addi	t0, t0, %pcrel_lo(.LBB12_19)
	sd	t0, 472(sp)
	sd	t2, 480(sp)
	addi	a4, sp, 504
	sd	a4, 488(sp)
	sd	a1, 496(sp)
	sd	t1, 440(sp)
	sd	a5, 448(sp)
	sd	zero, 424(sp)
	addi	a4, sp, 472
	sd	a4, 456(sp)
	sd	a0, 464(sp)
	sb	s2, 671(sp)
	sd	s0, 736(sp)
	addi	a4, sp, 736
	sd	a4, 720(sp)
.LBB12_20:
	auipc	a4, %pcrel_hi(_ZN44_$LT$$RF$T$u20$as$u20$core..fmt..Display$GT$3fmt17h9dbc321d37742d77E)
	addi	a4, a4, %pcrel_lo(.LBB12_20)
	sd	a4, 728(sp)
	sd	s1, 688(sp)
	sd	a7, 696(sp)
	sd	zero, 672(sp)
	addi	a4, sp, 720
	sd	a4, 704(sp)
	sd	a7, 712(sp)
	addi	a4, sp, 671
	sd	a4, 632(sp)
	sd	t2, 640(sp)
	addi	a4, sp, 672
	sd	a4, 648(sp)
	sd	a1, 656(sp)
	sd	t1, 600(sp)
	sd	a5, 608(sp)
	sd	zero, 584(sp)
	addi	a4, sp, 632
	sd	a4, 616(sp)
	sd	a0, 624(sp)
	addi	a4, sp, 200
	sd	a4, 120(sp)
.LBB12_21:
	auipc	a4, %pcrel_hi(_ZN4core3fmt3num3imp52_$LT$impl$u20$core..fmt..Display$u20$for$u20$u64$GT$3fmt17h5c6c68d79cd9e69aE)
	addi	a4, a4, %pcrel_lo(.LBB12_21)
	sd	a4, 128(sp)
	addi	a4, sp, 212
	sd	a4, 136(sp)
	sd	a3, 144(sp)
	addi	a3, sp, 216
	sd	a3, 152(sp)
	sd	a1, 160(sp)
	addi	a3, sp, 424
	sd	a3, 168(sp)
	sd	a1, 176(sp)
	addi	a3, sp, 584
	sd	a3, 184(sp)
	sd	a1, 192(sp)
	sd	a0, 744(sp)
	sd	zero, 760(sp)
	sd	a5, 768(sp)
	sd	a2, 776(sp)
	sb	a7, 784(sp)
	sd	zero, 792(sp)
	sd	a0, 800(sp)
	sd	zero, 816(sp)
	li	a3, 6
	sd	a3, 824(sp)
	addi	a4, a2, 8
	sd	a4, 832(sp)
	sb	a5, 840(sp)
	sd	a7, 848(sp)
	sd	a0, 856(sp)
	sd	a0, 872(sp)
	sd	a2, 888(sp)
	sb	a5, 896(sp)
	sd	a0, 904(sp)
	sd	a0, 912(sp)
	sd	a0, 928(sp)
	sd	a2, 944(sp)
	sb	a5, 952(sp)
	sd	a5, 960(sp)
	sd	a0, 968(sp)
	sd	a0, 984(sp)
	sd	a2, 1000(sp)
	sb	a5, 1008(sp)
	li	a2, 4
	sd	a2, 1016(sp)
.LBB12_22:
	auipc	a2, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.15)
	addi	a2, a2, %pcrel_lo(.LBB12_22)
	sd	a2, 88(sp)
	sd	a3, 96(sp)
	addi	a2, sp, 744
	sd	a2, 72(sp)
	sd	a6, 80(sp)
	addi	a2, sp, 120
	sd	a2, 104(sp)
	sd	a6, 112(sp)
	sd	t0, 40(sp)
	sd	t2, 48(sp)
	addi	a2, sp, 72
	sd	a2, 56(sp)
	sd	a1, 64(sp)
	addi	a1, sp, 1080
	sd	a1, 1024(sp)
	sd	zero, 1032(sp)
	sd	t1, 1048(sp)
	sd	a5, 1056(sp)
	addi	a1, sp, 40
	sd	a1, 1064(sp)
	sd	a0, 1072(sp)
.LBB12_23:
	auipc	a1, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.0)
	addi	a1, a1, %pcrel_lo(.LBB12_23)
	addi	a0, sp, 1024
	addi	a2, sp, 1032
	call	_ZN4core3fmt5write17hc94f11bd44ac9229E
	bnez	a0, .LBB12_9
	ld	ra, 1112(sp)
	ld	s0, 1104(sp)
	ld	s1, 1096(sp)
	ld	s2, 1088(sp)
	addi	sp, sp, 1120
	ret
.LBB12_9:
.LBB12_24:
	auipc	a0, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.20)
	addi	a0, a0, %pcrel_lo(.LBB12_24)
.LBB12_25:
	auipc	a3, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.21)
	addi	a3, a3, %pcrel_lo(.LBB12_25)
.LBB12_26:
	auipc	a4, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.23)
	addi	a4, a4, %pcrel_lo(.LBB12_26)
	li	a1, 43
	addi	a2, sp, 1080
	call	_ZN4core6result13unwrap_failed17h23d4566c50b348ceE
	unimp	
.Lfunc_end12:
	.size	_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$3log17h95b606852221f78fE, .Lfunc_end12-_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$3log17h95b606852221f78fE
	.section	".rodata._ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$3log17h95b606852221f78fE","a",@progbits
	.p2align	3
.LJTI12_0:
	.quad	.LBB12_3
	.quad	.LBB12_7
	.quad	.LBB12_4
	.quad	.LBB12_5
	.quad	.LBB12_6

	.section	".text._ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$5flush17h51488e7894d30454E","ax",@progbits
	.globl	_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$5flush17h51488e7894d30454E
	.p2align	1
	.type	_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$5flush17h51488e7894d30454E,@function
_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$5flush17h51488e7894d30454E:
	ret
.Lfunc_end13:
	.size	_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$5flush17h51488e7894d30454E, .Lfunc_end13-_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$5flush17h51488e7894d30454E

	.section	.text._ZN5axlog12__print_impl17h6f03fbb6b77d3807E,"ax",@progbits
	.globl	_ZN5axlog12__print_impl17h6f03fbb6b77d3807E
	.p2align	1
	.type	_ZN5axlog12__print_impl17h6f03fbb6b77d3807E,@function
_ZN5axlog12__print_impl17h6f03fbb6b77d3807E:
	addi	sp, sp, -80
	sd	ra, 72(sp)
	ld	a1, 40(a0)
	ld	a2, 32(a0)
	addi	a3, sp, 64
	sd	a3, 8(sp)
	sd	a1, 56(sp)
	sd	a2, 48(sp)
	ld	a1, 24(a0)
	ld	a2, 16(a0)
	ld	a3, 8(a0)
	ld	a0, 0(a0)
	sd	a1, 40(sp)
	sd	a2, 32(sp)
	sd	a3, 24(sp)
	sd	a0, 16(sp)
.LBB14_3:
	auipc	a1, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.0)
	addi	a1, a1, %pcrel_lo(.LBB14_3)
	addi	a0, sp, 8
	addi	a2, sp, 16
	call	_ZN4core3fmt5write17hc94f11bd44ac9229E
	bnez	a0, .LBB14_2
	ld	ra, 72(sp)
	addi	sp, sp, 80
	ret
.LBB14_2:
.LBB14_4:
	auipc	a0, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.20)
	addi	a0, a0, %pcrel_lo(.LBB14_4)
.LBB14_5:
	auipc	a3, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.21)
	addi	a3, a3, %pcrel_lo(.LBB14_5)
.LBB14_6:
	auipc	a4, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.23)
	addi	a4, a4, %pcrel_lo(.LBB14_6)
	li	a1, 43
	addi	a2, sp, 64
	call	_ZN4core6result13unwrap_failed17h23d4566c50b348ceE
	unimp	
.Lfunc_end14:
	.size	_ZN5axlog12__print_impl17h6f03fbb6b77d3807E, .Lfunc_end14-_ZN5axlog12__print_impl17h6f03fbb6b77d3807E

	.section	.text._ZN5axlog4init17h1202923b1ab04d47E,"ax",@progbits
	.globl	_ZN5axlog4init17h1202923b1ab04d47E
	.p2align	1
	.type	_ZN5axlog4init17h1202923b1ab04d47E,@function
_ZN5axlog4init17h1202923b1ab04d47E:
	addi	sp, sp, -16
	sd	ra, 8(sp)
.LBB15_3:
	auipc	a0, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.3)
	addi	a0, a0, %pcrel_lo(.LBB15_3)
.LBB15_4:
	auipc	a1, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.25)
	addi	a1, a1, %pcrel_lo(.LBB15_4)
	call	_ZN3log10set_logger17h5cab20171868d9e2E
	bnez	a0, .LBB15_2
.LBB15_5:
	auipc	a0, %pcrel_hi(_ZN3log20MAX_LOG_LEVEL_FILTER17h1e26f6a447dac69eE)
	addi	a0, a0, %pcrel_lo(.LBB15_5)
	li	a1, 2
	sd	a1, 0(a0)
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
.LBB15_2:
.LBB15_6:
	auipc	a0, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.20)
	addi	a0, a0, %pcrel_lo(.LBB15_6)
.LBB15_7:
	auipc	a3, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.26)
	addi	a3, a3, %pcrel_lo(.LBB15_7)
.LBB15_8:
	auipc	a4, %pcrel_hi(.Lanon.1c9a65c644e766cfc593bf46d26904c7.27)
	addi	a4, a4, %pcrel_lo(.LBB15_8)
	li	a1, 43
	mv	a2, sp
	call	_ZN4core6result13unwrap_failed17h23d4566c50b348ceE
	unimp	
.Lfunc_end15:
	.size	_ZN5axlog4init17h1202923b1ab04d47E, .Lfunc_end15-_ZN5axlog4init17h1202923b1ab04d47E

	.section	.text._ZN5axlog13set_max_level17h16eab4f7fe8870b2E,"ax",@progbits
	.globl	_ZN5axlog13set_max_level17h16eab4f7fe8870b2E
	.p2align	1
	.type	_ZN5axlog13set_max_level17h16eab4f7fe8870b2E,@function
_ZN5axlog13set_max_level17h16eab4f7fe8870b2E:
	addi	sp, sp, -16
	sd	ra, 8(sp)
	call	_ZN63_$LT$log..LevelFilter$u20$as$u20$core..str..traits..FromStr$GT$8from_str17hb3dfe60343858090E
	li	a2, 6
	li	a1, 0
	beq	a0, a2, .LBB16_2
	mv	a1, a0
.LBB16_2:
.LBB16_3:
	auipc	a0, %pcrel_hi(_ZN3log20MAX_LOG_LEVEL_FILTER17h1e26f6a447dac69eE)
	addi	a0, a0, %pcrel_lo(.LBB16_3)
	sd	a1, 0(a0)
	ld	ra, 8(sp)
	addi	sp, sp, 16
	ret
.Lfunc_end16:
	.size	_ZN5axlog13set_max_level17h16eab4f7fe8870b2E, .Lfunc_end16-_ZN5axlog13set_max_level17h16eab4f7fe8870b2E

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.0,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.0,"a",@progbits
	.p2align	3
.Lanon.1c9a65c644e766cfc593bf46d26904c7.0:
	.quad	_ZN4core3ptr46drop_in_place$LT$$RF$mut$u20$axlog..Logger$GT$17heb065581dc1c6a20E
	.asciz	"\b\000\000\000\000\000\000\000\b\000\000\000\000\000\000"
	.quad	_ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$9write_str17h984b477899bf930cE
	.quad	_ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$10write_char17h75f71f3ea8095724E
	.quad	_ZN50_$LT$$RF$mut$u20$W$u20$as$u20$core..fmt..Write$GT$9write_fmt17h56a9bcec95552620E
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.0, 48

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.1,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.1,"a",@progbits
.Lanon.1c9a65c644e766cfc593bf46d26904c7.1:
	.ascii	"invalid args"
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.1, 12

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.2,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.2,"a",@progbits
	.p2align	3
.Lanon.1c9a65c644e766cfc593bf46d26904c7.2:
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.1
	.asciz	"\f\000\000\000\000\000\000"
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.2, 16

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.3,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.3,"a",@progbits
	.p2align	3
.Lanon.1c9a65c644e766cfc593bf46d26904c7.3:
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.3, 0

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.4,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.4,"a",@progbits
.Lanon.1c9a65c644e766cfc593bf46d26904c7.4:
	.ascii	"/home/cloud/.rustup/toolchains/nightly-x86_64-unknown-linux-gnu/lib/rustlib/src/rust/library/core/src/fmt/mod.rs"
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.4, 112

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.5,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.5,"a",@progbits
	.p2align	3
.Lanon.1c9a65c644e766cfc593bf46d26904c7.5:
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.4
	.asciz	"p\000\000\000\000\000\000\000\223\001\000\000\r\000\000"
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.5, 24

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.6,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.6,"a",@progbits
.Lanon.1c9a65c644e766cfc593bf46d26904c7.6:
	.ascii	"\033["
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.6, 2

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.7,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.7,"a",@progbits
.Lanon.1c9a65c644e766cfc593bf46d26904c7.7:
	.byte	109
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.7, 1

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.8,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.8,"a",@progbits
.Lanon.1c9a65c644e766cfc593bf46d26904c7.8:
	.ascii	"\033[m"
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.8, 3

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.9,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.9,"a",@progbits
	.p2align	3
.Lanon.1c9a65c644e766cfc593bf46d26904c7.9:
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.6
	.asciz	"\002\000\000\000\000\000\000"
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.7
	.asciz	"\001\000\000\000\000\000\000"
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.8
	.asciz	"\003\000\000\000\000\000\000"
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.9, 48

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.10,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.10,"a",@progbits
.Lanon.1c9a65c644e766cfc593bf46d26904c7.10:
	.byte	37
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.10, 1

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.11,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.11,"a",@progbits
.Lanon.1c9a65c644e766cfc593bf46d26904c7.11:
	.byte	91
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.11, 1

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.12,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.12,"a",@progbits
.Lanon.1c9a65c644e766cfc593bf46d26904c7.12:
	.byte	46
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.12, 1

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.13,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.13,"a",@progbits
.Lanon.1c9a65c644e766cfc593bf46d26904c7.13:
	.byte	32
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.13, 1

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.14,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.14,"a",@progbits
.Lanon.1c9a65c644e766cfc593bf46d26904c7.14:
	.byte	10
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.14, 1

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.15,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.15,"a",@progbits
	.p2align	3
.Lanon.1c9a65c644e766cfc593bf46d26904c7.15:
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.11
	.asciz	"\001\000\000\000\000\000\000"
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.12
	.asciz	"\001\000\000\000\000\000\000"
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.13
	.asciz	"\001\000\000\000\000\000\000"
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.13
	.asciz	"\001\000\000\000\000\000\000"
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.13
	.asciz	"\001\000\000\000\000\000\000"
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.14
	.asciz	"\001\000\000\000\000\000\000"
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.15, 96

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.16,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.16,"a",@progbits
	.p2align	3
.Lanon.1c9a65c644e766cfc593bf46d26904c7.16:
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.3
	.zero	8
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.16, 16

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.17,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.17,"a",@progbits
.Lanon.1c9a65c644e766cfc593bf46d26904c7.17:
	.byte	58
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.17, 1

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.18,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.18,"a",@progbits
.Lanon.1c9a65c644e766cfc593bf46d26904c7.18:
	.byte	93
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.18, 1

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.19,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.19,"a",@progbits
	.p2align	3
.Lanon.1c9a65c644e766cfc593bf46d26904c7.19:
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.3
	.zero	8
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.17
	.asciz	"\001\000\000\000\000\000\000"
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.18
	.asciz	"\001\000\000\000\000\000\000"
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.19, 48

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.20,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.20,"a",@progbits
.Lanon.1c9a65c644e766cfc593bf46d26904c7.20:
	.ascii	"called `Result::unwrap()` on an `Err` value"
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.20, 43

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.21,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.21,"a",@progbits
	.p2align	3
.Lanon.1c9a65c644e766cfc593bf46d26904c7.21:
	.quad	_ZN4core3ptr37drop_in_place$LT$core..fmt..Error$GT$17hd32a9656971dfae9E
	.asciz	"\000\000\000\000\000\000\000\000\001\000\000\000\000\000\000"
	.quad	_ZN53_$LT$core..fmt..Error$u20$as$u20$core..fmt..Debug$GT$3fmt17h96319b6abba65cfdE
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.21, 32

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.22,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.22,"a",@progbits
.Lanon.1c9a65c644e766cfc593bf46d26904c7.22:
	.ascii	"modules/axlog/src/lib.rs"
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.22, 24

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.23,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.23,"a",@progbits
	.p2align	3
.Lanon.1c9a65c644e766cfc593bf46d26904c7.23:
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.22
	.asciz	"\030\000\000\000\000\000\000\000u\000\000\000\034\000\000"
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.23, 24

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.24,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.24,"a",@progbits
	.p2align	3
.Lanon.1c9a65c644e766cfc593bf46d26904c7.24:
	.quad	_ZN4core3ptr34drop_in_place$LT$axlog..Logger$GT$17h828d12fb3f838e82E
	.asciz	"\000\000\000\000\000\000\000\000\001\000\000\000\000\000\000"
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.24, 24

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.25,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.25,"a",@progbits
	.p2align	3
.Lanon.1c9a65c644e766cfc593bf46d26904c7.25:
	.quad	_ZN4core3ptr34drop_in_place$LT$axlog..Logger$GT$17h828d12fb3f838e82E
	.asciz	"\000\000\000\000\000\000\000\000\001\000\000\000\000\000\000"
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.24
	.quad	_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$7enabled17hb5d3b6f14a3b2756E
	.quad	_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$3log17h95b606852221f78fE
	.quad	_ZN42_$LT$axlog..Logger$u20$as$u20$log..Log$GT$5flush17h51488e7894d30454E
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.25, 56

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.26,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.26,"a",@progbits
	.p2align	3
.Lanon.1c9a65c644e766cfc593bf46d26904c7.26:
	.quad	_ZN4core3ptr40drop_in_place$LT$log..SetLoggerError$GT$17hba5946db384333f1E
	.asciz	"\000\000\000\000\000\000\000\000\001\000\000\000\000\000\000"
	.quad	_ZN56_$LT$log..SetLoggerError$u20$as$u20$core..fmt..Debug$GT$3fmt17h23dbdd424872c4c4E
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.26, 32

	.type	.Lanon.1c9a65c644e766cfc593bf46d26904c7.27,@object
	.section	.rodata..Lanon.1c9a65c644e766cfc593bf46d26904c7.27,"a",@progbits
	.p2align	3
.Lanon.1c9a65c644e766cfc593bf46d26904c7.27:
	.quad	.Lanon.1c9a65c644e766cfc593bf46d26904c7.22
	.asciz	"\030\000\000\000\000\000\000\000y\000\000\000\036\000\000"
	.size	.Lanon.1c9a65c644e766cfc593bf46d26904c7.27, 24

	.section	".note.GNU-stack","",@progbits
