# options
#BUILD_STRATEGY	:= htlatex

neverclean := *.pdf
onlysources.tex := relatorio.tex
#onlysources.tex := relatorio.tex apresentacao.tex

.PHONY:
all: all-pdf

-include latex.make


# UTILS
#
.PHONY:
todos:
	@grep -rn "TODO" *.tex partes


#
# vim: noet sts=0 sw=8 ts=8 filetype=make
