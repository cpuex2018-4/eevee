main:
#	main program starts
	fli	fa0, L_a	# 0.9647326178866098
#	fli	fa0, L_1	# 0.5403023058681398
#	fli	fa0, L_2	# -0.4161468365471424
	call	min_caml_cos
#	main program ends
end:
	b	end
	.data
L_a:	# 12.3
	.word	1095027917
