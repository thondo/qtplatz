// This is a -*- C++ -*- header.
/**************************************************************************
** Copyright (C) 2010-2013 Toshinobu Hondo, Ph.D.
** Copyright (C) 2013 MS-Cheminformatics LLC
*
** Contact: info@ms-cheminfo.com
**
** Commercial Usage
**
** Licensees holding valid MS-Cheminformatics commercial licenses may use this file in
** accordance with the MS-Cheminformatics Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and MS-Cheminformatics.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.TXT included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
**************************************************************************/

#pragma once

#include <string>
#include <map>
#include <vector>

namespace adportable {

#if 0
    class Module {
    public:
        Module( const std::wstring& xml = L"" );
        Module( const Module& );
        inline const std::wstring& xml() const { return xml_; }
        inline const std::wstring& library_filename() const { return library_filename_; }
        inline const std::string& object_reference() const { return object_reference_; }
        inline const std::string& id() const { return id_; }
        void xml( const std::wstring& );
        void library_filename( const std::wstring& );
        void library_filename( const std::string& );
        void object_reference( const std::string& );
        void id( const std::string& );

    private:
        std::wstring xml_;
        std::wstring library_filename_;
        std::string object_reference_;
        std::string id_;
    };
#endif

    class Configuration {
    public:
        ~Configuration(void);
        Configuration(void);
        Configuration( const Configuration& );

        typedef std::vector<Configuration> vector_type;
        typedef std::map<std::wstring, std::wstring> attributes_type;

        const std::wstring& component() const;

        const std::wstring& _interface() const;
        void _interface( const std::wstring& );
		
        const std::wstring& attribute( const std::wstring& key ) const;
		
        const std::wstring& name() const;
        void name( const std::wstring& );
		
        const std::wstring& title() const;
        void title( const std::wstring& );
		
        void attribute( const std::wstring& key, const std::wstring& value );
        bool readonly() const;
        bool hasChild() const;
        Configuration& append( const Configuration& );
        void xml( const std::wstring& );
        // void module( const Module& );
		
        // bool isComponent() const;
        // bool isPlugin() const;
		
        inline const std::wstring& xml() const { return xml_; }
        inline const attributes_type& attributes() const { return attributes_; }
        // inline const Module& module() const { return module_; }
        inline vector_type::iterator begin() { return children_.begin(); }
        inline vector_type::iterator end()   { return children_.end(); }
        inline vector_type::reverse_iterator rbegin() { return children_.rbegin(); }
        inline vector_type::reverse_iterator rend()   { return children_.rend(); }
        inline vector_type::const_iterator begin() const { return children_.begin(); }
        inline vector_type::const_iterator end() const  { return children_.end(); }
		
        static const Configuration * find( const Configuration&, const std::wstring& );
		
    private:
        std::wstring xml_;
        std::wstring name_;
        std::wstring text_;
        std::wstring interface_;  // <Component interface="value"/>
        attributes_type attributes_;
        std::vector< Configuration > children_;
    };

}
