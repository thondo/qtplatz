INCLUDEPATH += $$(ACE_ROOT) $$(TAO_ROOT)
LIBS += -L$$(ACE_ROOT)/lib \
        -l$$qtLibraryTarget(ACE) \
        -l$$qtLibraryTarget(TAO)
