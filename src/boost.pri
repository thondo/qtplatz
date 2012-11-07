BOOST_VERSION=boost-1_52

win32 {
      BOOST = $$(BOOST_ROOT)
      isEmpty( BOOST ) BOOST = C:/Boost
      INCLUDEPATH += $${BOOST}/include/$${BOOST_VERSION}
      LIBS += -L$${BOOST}\\lib
} else {
      INCLUDEPATH += /usr/local/$${BOOST_VERSION}/include
      QMAKE_LFLAGS += -L/usr/local/$${BOOST_VERSION}/lib
}

