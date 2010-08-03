GOC		= gccgo -g
GOFLAGS		= -O -Wall

PROGS		= args goecho sieve

all:		$(PROGS)

clean:
		rm -f *~ $(PROGS) a.out

args:		args.go
		$(GOC) $< -o $@

goecho:		goecho.go
		$(GOC) $< -o $@

sieve:		sieve.go
		$(GOC) $< -o $@
