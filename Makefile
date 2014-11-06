OPEN = $(shell which -s gnome-open && echo gnome-open \
            || which -s xdg-open && echo xdg-open \
            || which -s open && echo open)


.PHONY: all
all: relatorio.pdf apresentacao.pdf

neverclean := *.pdf
-include latex.make


# UTILS
#

.PHONY: open
open:
	$(OPEN) relatorio.pdf

.PHONY: todos
todos:
	@grep -rn "\(TODO\|XXX\|FIXME\)" *.tex doc


#
# vim: noet sts=0 sw=8 ts=8 filetype=make
