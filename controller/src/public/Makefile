#
# Makefile for public src
#

styles.css: styles.scss
	npx sass $^ $@

all: styles.css


watch:
	npx sass --watch styles.scss:styles.css

clean:
	rm -fr styles.css
