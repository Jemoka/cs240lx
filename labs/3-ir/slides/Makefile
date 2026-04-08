NAME = 3-ir-notes

TARGET = $(NAME).pdf 
ifneq ($(RUN),0)
    TARGET += $(NAME).run
endif

all: $(TARGET)


%.pdf: %.md
	marp  --allow-local-files  --theme narrow-bullets.css --html $*.md -o $*.pdf

%.run: %.pdf
	evince $<

clean:
	rm -f *~

.PHONEY: $(NAME).run
