// This is a -*- C++ -*- header.
//////////////////////////////////////////
// Copyright (C) 2010 Toshinobu Hondo, Ph.D.
// Science Liaison / Advanced Instrumentation Project
//////////////////////////////////////////

#ifndef SPECTRUMWIDGET_H
#define SPECTRUMWIDGET_H

// #include <adwidgets/dataplotwidget.h>
#include "tracewidget.h"

namespace adwidgets {
    namespace ui {

        class SpectrumWidget : public TraceWidget {
            Q_OBJECT
        public:
			~SpectrumWidget();
            explicit SpectrumWidget(QWidget *parent = 0);

        signals:

        public slots:

        };

    }
}

#endif // SPECTRUMWIDGET_H
