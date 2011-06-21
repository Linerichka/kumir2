include(../../kumir2.pri)

TEMPLATE = app
TARGET = phony_target
CONFIG -= qt separate_debug_info gdb_dwarf_index
QT =
LIBS =
macx:CONFIG -= app_bundle

isEmpty(vcproj) {
    QMAKE_LINK = @: IGNORE THIS LINE
    OBJECTS_DIR =
    win32:CONFIG -= embed_manifest_exe
} else {
    CONFIG += console
    PHONY_DEPS = .
    phony_src.input = PHONY_DEPS
    phony_src.output = phony.c
    phony_src.variable_out = GENERATED_SOURCES
    phony_src.commands = echo int main() { return 0; } > phony.c
    phony_src.name = CREATE phony.c
    phony_src.CONFIG += combine
    QMAKE_EXTRA_COMPILERS += phony_src
}

DATA_DIRS = kumiranalizer kumircppgenerator coregui editor

!isEmpty(copydata) {

    for(data_dir, DATA_DIRS) {
        files = $$files($$PWD/$$data_dir/*, true)
        win32:files ~= s|\\\\|/|g
        for(file, files):!exists($$file/*):FILES += $$file
    }
    

    copy2build.input = FILES
    copy2build.output = $$IDE_DATA_PATH/${QMAKE_FUNC_FILE_IN_stripSrcDir}
    isEmpty(vcproj):copy2build.variable_out = PRE_TARGETDEPS
    win32:copy2build.commands = $$QMAKE_COPY \"${QMAKE_FILE_IN}\" \"${QMAKE_FILE_OUT}\"
    unix:copy2build.commands = $$QMAKE_COPY ${QMAKE_FILE_IN} ${QMAKE_FILE_OUT}
    copy2build.name = COPY ${QMAKE_FILE_IN}
    copy2build.CONFIG += no_link
    QMAKE_EXTRA_COMPILERS += copy2build

}

system(python translations/run_lrelease.py $$PWD/translations $$IDE_DATA_PATH/translations)

!macx {
    for(data_dir, DATA_DIRS) {
        eval($${data_dir}.files = $$quote($$PWD/$$data_dir))
        eval($${data_dir}.path = /share/kumir2)
        INSTALLS += $$data_dir
    }
}
