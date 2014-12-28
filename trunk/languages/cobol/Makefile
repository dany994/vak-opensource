COBC		= cobc -x

all:		fact

clean:
		rm -f *~ fact

fact:		fact.cob
		$(COBC) $< -o $@
