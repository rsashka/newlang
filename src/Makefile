#
#  There exist several targets which are by default empty and which can be 
#  used for execution of your targets. These targets are usually executed 
#  before and after some main targets. They are: 
#
#     .build-pre:              called before 'build' target
#     .build-post:             called after 'build' target
#     .clean-pre:              called before 'clean' target
#     .clean-post:             called after 'clean' target
#     .clobber-pre:            called before 'clobber' target
#     .clobber-post:           called after 'clobber' target
#     .all-pre:                called before 'all' target
#     .all-post:               called after 'all' target
#     .help-pre:               called before 'help' target
#     .help-post:              called after 'help' target
#
#  Targets beginning with '.' are not intended to be called on their own.
#
#  Main targets can be executed directly, and they are:
#  
#     build                    build a specific configuration
#     clean                    remove built files from a configuration
#     clobber                  remove all built files
#     all                      build all configurations
#     help                     print help mesage
#  
#  Targets .build-impl, .clean-impl, .clobber-impl, .all-impl, and
#  .help-impl are implemented in nbproject/makefile-impl.mk.
#
#  Available make variables:
#
#     CND_BASEDIR                base directory for relative paths
#     CND_DISTDIR                default top distribution directory (build artifacts)
#     CND_BUILDDIR               default top build directory (object files, ...)
#     CONF                       name of current configuration
#     CND_PLATFORM_${CONF}       platform name (current configuration)
#     CND_ARTIFACT_DIR_${CONF}   directory of build artifact (current configuration)
#     CND_ARTIFACT_NAME_${CONF}  name of build artifact (current configuration)
#     CND_ARTIFACT_PATH_${CONF}  path to build artifact (current configuration)
#     CND_PACKAGE_DIR_${CONF}    directory of package (current configuration)
#     CND_PACKAGE_NAME_${CONF}   name of package (current configuration)
#     CND_PACKAGE_PATH_${CONF}   path to package (current configuration)
#
# NOCDDL


# Environment 
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin


#VERSION_MAJOR=0
#VERSION_MINOR=3
#VERSION_PATCH=0
#VERSION_HEADER="version.h"
#VERSION_FILE="version.cpp"
#VERSION_HEADER_TEMP="version.h.temp"
#VERSION_FILE_TEMP="version.cpp.temp"
#
#
#GIT_TAG_VERSION=$(shell git describe --abbrev=0 --tags)
#GIT_SHORT_HASH=$(shell git rev-parse --short HEAD)
#GIT_SOURCE_ID=$(GIT_TAG_VERSION)-$(GIT_SHORT_HASH)
#DATE_BUILD=$(shell date +'%y.%m.%d %H:%M:%S')

# build
build: .build-post

.build-pre:
# Add your pre 'build' code here...

	../build_doc.sh ..
#
#	@cat ../docs/syntax.md > ../docs/newlang_doc.md
#	@echo "---\nsyntax.md\n\n" >> ../docs/newlang_doc.md
#	@cat ../docs/types.md >> ../docs/newlang_doc.md
#	@echo "---\ntypes.md\n\n" >> ../docs/newlang_doc.md
#	@cat ../docs/type_nor.md >> ../docs/newlang_doc.md
#	@echo "---\ntype_nor.md\n\n" >> ../docs/newlang_doc.md
#	@cat ../docs/type_str.md >> ../docs/newlang_doc.md
#	@echo "---\ntype_str.md\n\n" >> ../docs/newlang_doc.md
#	@cat ../docs/type_func.md >> ../docs/newlang_doc.md
#	@echo "---\ntype_func.md\n\n" >> ../docs/newlang_doc.md
#	@cat ../docs/type_oop.md >> ../docs/newlang_doc.md
#	@echo "---\ntype_oop.md\n\n" >> ../docs/newlang_doc.md
#	@cat ../docs/type_native.md >> ../docs/newlang_doc.md
#	@echo "---\ntype_native.md\n\n" >> ../docs/newlang_doc.md
#	@cat ../docs/type_iter.md >> ../docs/newlang_doc.md
#	@echo "---\ntype_iter.md\n\n" >> ../docs/newlang_doc.md
#	@cat ../docs/type_other.md >> ../docs/newlang_doc.md
#	@echo "---\ntype_other.md\n\n" >> ../docs/newlang_doc.md
#	@cat ../docs/ops.md >> ../docs/newlang_doc.md
#	@echo "---\nops.md\n\n" >> ../docs/newlang_doc.md
#	@cat ../docs/syntax_dsl.md >> ../docs/newlang_doc.md
#	@echo "---\nsyntax_dsl.md\n\n" >> ../docs/newlang_doc.md
#	
#	#pandoc -t plain ../docs/newlang_doc.md > ../docs/newlang_doc.txt
#	../contrib/text2cpp/output/bin/text2cpp ../docs/newlang_doc.md  syntax_help.cpp.temp newlang_syntax_help c
#
#	@if cmp -s syntax_help.cpp.temp syntax_help.cpp; then \
#	    rm syntax_help.cpp.temp; \
#	    echo Use file syntax_help.cpp for current build; \
#	else \
#	    rm syntax_help.cpp; \
#	    mv syntax_help.cpp.temp  syntax_help.cpp; \
#	    echo Make file syntax_help.cpp for version $(GIT_SOURCE_ID) at date $(DATE_BUILD); \
#	fi
#	
#	../contrib/text2cpp/output/bin/text2cpp ../examples/dsl.nlp  dsl.cpp.temp newlang_dsl c
#
#	@if cmp -s dsl.cpp.temp dsl.cpp; then \
#	    rm dsl.cpp.temp; \
#	    echo Use file dsl.cpp for current build; \
#	else \
#	    rm dsl.cpp; \
#	    mv dsl.cpp.temp  dsl.cpp; \
#	    echo Make file dsl.cpp for version $(GIT_SOURCE_ID) at date $(DATE_BUILD); \
#	fi
#
#	@if [ "$(GIT_TAG_VERSION)" != "v$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)" ]; then \
#	    echo "Git TAG $(GIT_TAG_VERSION) differ version v$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)"; \
#	    exit 1; \
#	fi
#	@if [ -z $(GIT_SHORT_HASH) ]; then \
#	    echo Undefined version for build; \
#	    exit 1; \
#	fi
#	@if [ -f $(VERSION_HEADER_TEMP) ]; then \
#	    rm $(VERSION_HEADER_TEMP); \
#	fi
#
#	@echo "/** @file $(VERSION_HEADER)" > $(VERSION_HEADER_TEMP)
#	@echo "* Auto generate file for identification current build" >> $(VERSION_HEADER_TEMP)
#	@echo "* Date build $(DATE_BUILD)" >> $(VERSION_HEADER_TEMP)
#	@echo "*/"  >> $(VERSION_HEADER_TEMP)
#	@echo ""  >> $(VERSION_HEADER_TEMP)
#
#	@echo "#define VERSION_DATE_BUILD_STR \"$(DATE_BUILD)\""  >> $(VERSION_HEADER_TEMP)
#	@echo "#define VERSION_SOURCE_FULL_ID \"$(GIT_SOURCE_ID) $(DATE_BUILD)\""  >> $(VERSION_HEADER_TEMP)
#	@echo ""  >> $(VERSION_HEADER_TEMP)
#
#	@echo "#include <stdint.h>"  >> $(VERSION_HEADER_TEMP)
#
#	@echo "extern const uint8_t VERSION_MAJOR;"  >> $(VERSION_HEADER_TEMP)
#	@echo "extern const uint8_t VERSION_MINOR;"  >> $(VERSION_HEADER_TEMP)
#	@echo "extern const uint8_t VERSION_PATCH;"  >> $(VERSION_HEADER_TEMP)
#	@echo "extern const uint16_t VERSION_BUILD;"  >> $(VERSION_HEADER_TEMP)
#	@echo ""  >> $(VERSION_HEADER_TEMP)
#
#	@echo "#define VERSION ($(VERSION_MAJOR) << 4 | $(VERSION_MINOR))"  >> $(VERSION_HEADER_TEMP)
#	@echo "#define VERSION_GIT_SOURCE \"$(GIT_SOURCE_ID)\""  >> $(VERSION_HEADER_TEMP)
#	@echo ""  >> $(VERSION_HEADER_TEMP)
#
#	@echo "extern const char * GIT_SOURCE;"  >> $(VERSION_HEADER_TEMP)
#	@echo "extern const char * DATE_BUILD_STR;"  >> $(VERSION_HEADER_TEMP)
#	@echo "extern const char * SOURCE_FULL_ID; "  >> $(VERSION_HEADER_TEMP)
#	@echo ""  >> $(VERSION_HEADER_TEMP)
#
#	@if [ -f $(VERSION_FILE_TEMP) ]; then \
#	    rm $(VERSION_FILE_TEMP); \
#	fi
#
#	@echo "/** @file $(VERSION_FILE)" > $(VERSION_FILE_TEMP)
#	@echo "* Auto generate file for identification current build" >> $(VERSION_FILE_TEMP)
#	@echo "* Date build $(DATE_BUILD)" >> $(VERSION_FILE_TEMP)
#	@echo "*/"  >> $(VERSION_FILE_TEMP)
#	@echo ""  >> $(VERSION_FILE_TEMP)
#	
#	@echo "#include \"version.h\""  >> $(VERSION_FILE_TEMP)
#	@echo ""  >> $(VERSION_FILE_TEMP)
#	
#	@echo "const uint8_t VERSION_MAJOR=$(VERSION_MAJOR);"  >> $(VERSION_FILE_TEMP)
#	@echo "const uint8_t VERSION_MINOR=$(VERSION_MINOR);"  >> $(VERSION_FILE_TEMP)
#	@echo "const uint8_t VERSION_PATCH=$(VERSION_PATCH);"  >> $(VERSION_FILE_TEMP)
#	@echo ""  >> $(VERSION_FILE_TEMP)
#	
#	@echo "const char * GIT_SOURCE = VERSION_GIT_SOURCE;"  >> $(VERSION_FILE_TEMP)
#	@echo "const char * DATE_BUILD_STR = VERSION_DATE_BUILD_STR;"  >> $(VERSION_FILE_TEMP)
#	@echo "const char * SOURCE_FULL_ID = VERSION_SOURCE_FULL_ID;"  >> $(VERSION_FILE_TEMP)
#	@echo ""  >> $(VERSION_FILE_TEMP)
#
#	@if cmp --ignore-initial=250:250 -s $(VERSION_HEADER) $(VERSION_HEADER_TEMP); then \
#	    rm $(VERSION_HEADER_TEMP); \
#	    echo Use file $(VERSION_HEADER) for current build; \
#	else \
#	    rm $(VERSION_HEADER); \
#	    mv $(VERSION_HEADER_TEMP) $(VERSION_HEADER); \
#	    echo Make file $(VERSION_HEADER) for version $(GIT_SOURCE_ID) at date $(DATE_BUILD); \
#	fi
#	
#	@if cmp --ignore-initial=110:110 -s $(VERSION_FILE) $(VERSION_FILE_TEMP); then \
#	    rm $(VERSION_FILE_TEMP); \
#	    echo Use file $(VERSION_FILE) for current build; \
#	else \
#	    rm $(VERSION_FILE); \
#	    mv $(VERSION_FILE_TEMP) $(VERSION_FILE); \
#	    echo Make file $(VERSION_FILE) for version $(GIT_SOURCE_ID) at date $(DATE_BUILD); \
#	fi



.build-post: .build-impl
# Add your post 'build' code here...


# clean
clean: .clean-post

.clean-pre:
# Add your pre 'clean' code here...

.clean-post: .clean-impl
# Add your post 'clean' code here...


# clobber
clobber: .clobber-post

.clobber-pre:
# Add your pre 'clobber' code here...

.clobber-post: .clobber-impl
# Add your post 'clobber' code here...


# all
all: .all-post

.all-pre:
# Add your pre 'all' code here...

.all-post: .all-impl
# Add your post 'all' code here...


# build tests
build-tests: .build-tests-post

.build-tests-pre:
# Add your pre 'build-tests' code here...

.build-tests-post: .build-tests-impl
# Add your post 'build-tests' code here...


# run tests
test: .test-post

.test-pre: build-tests
# Add your pre 'test' code here...

.test-post: .test-impl
# Add your post 'test' code here...


# help
help: .help-post

.help-pre:
# Add your pre 'help' code here...

.help-post: .help-impl
# Add your post 'help' code here...



# include project implementation makefile
include nbproject/Makefile-impl.mk

# include project make variables
include nbproject/Makefile-variables.mk
