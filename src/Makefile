# root makefile. Delegate to source subdirs.

PACKAGE=.

SUBDIRS	= g2d\
          g2d/jlambda\
          g2d/closure\
          g2d/glyph\
          g2d/graph\
	  g2d/graph/java_cup\
	  g2d/graph/java_cup/runtime\
          g2d/swing\
          g2d/util


all:
	@@for p in $(SUBDIRS); do \
		echo '----- building ' $(PACKAGE)/$$p; \
		make -C $(PACKAGE)/$$p --no-print-directory  all; \
	done

clean:
	@@for p in $(SUBDIRS); do \
		echo 'cleaning ' $(PACKAGE)/$$p; \
		make -C $(PACKAGE)/$$p --no-print-directory clean;\
	done

doc_local:
	ant javadoc
#	javadoc -source 1.4 \
#		-author \
#		-use \
#		-public \
#		-splitIndex \
#		-d doc \
#		-doctitle 'Graphics2D Actor  API' \
#		-windowtitle 'Graphics2D Actor  API' \
#		-header 'Graphics2D Actor  API' \
#		-link file:///usr/java/java/docs/api \
#		g2d g2d.closure g2d.glyph g2d.jlambda g2d.graph g2d.swing g2d.util

doc_www: doc_local
#	javadoc -source 1.4 \
#		-author \
#		-use \
#		-public \
#		-splitIndex \
#		-d /tmp/iopdoc \
#		-doctitle 'Graphics2D Actor  API' \
#		-windowtitle 'Graphics2D Actor  API' \
#		-header 'Graphics2D Actor  API' \
#		-link http://mcs.une.edu.au/doc/java/api \
#		g2d g2d.closure g2d.glyph g2d.jlambda g2d.graph g2d.swing g2d.util

checkstyle:
#	java -cp /usr/local/checkstyle-3.5/checkstyle-all-3.5.jar com.puppycrawl.tools.checkstyle.Main -c iop.xml -r g2d/jlambda ; echo "finished g2d/jlambda."
#	java -cp /usr/local/checkstyle-3.5/checkstyle-all-3.5.jar com.puppycrawl.tools.checkstyle.Main -c iop.xml -r g2d/util ; echo "finished g2d/util."	
#	java -cp /usr/local/checkstyle-3.5/checkstyle-all-3.5.jar com.puppycrawl.tools.checkstyle.Main -c iop.xml -r g2d/glyph ; echo "finished g2d/glyph."	
#	java -cp /usr/local/checkstyle-3.5/checkstyle-all-3.5.jar com.puppycrawl.tools.checkstyle.Main -c iop.xml -r g2d/closure ; echo "finished g2d/closure."	
#	java -cp /usr/local/checkstyle-3.5/checkstyle-all-3.5.jar com.puppycrawl.tools.checkstyle.Main -c iop.xml -r g2d/swing ; echo "finished g2d/swing."	

release:
	cp -r classes/g2d jlam/
	cp -r doc jlam/
	cp -r JLambda jlam/
	zip -r jlam jlam

