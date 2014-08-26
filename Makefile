

.PHONY:
all: relatorio.pdf


neverclean := *.pdf
-include latex.make


# UTILS
#
.PHONY:
todos:
	@grep -rn "TODO" *.tex partes


#
# vim: noet sts=0 sw=8 ts=8 filetype=make
