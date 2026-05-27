NAME = 16-eraser-notes

all:
	marp  --allow-local-files --html $(NAME).md -o $(NAME).pdf
	evince $(NAME).pdf

clean:
	rm -f $(NAME).pdf *~
