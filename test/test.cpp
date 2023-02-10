// test.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "test.h"

#ifdef _DEBUG
#pragma comment( lib , "uppcored.lib" )
#pragma comment( lib , "uppctrlcored.lib" )
#pragma comment( lib , "uppctrllibd.lib" )
#pragma comment( lib , "uppdrawd.lib" )
#pragma comment( lib , "upprichtextd.lib" )
#pragma comment( lib , "uppricheditd.lib" )
#pragma comment( lib , "uppreportd.lib" )
#pragma comment( lib , "zlibd.lib" )
#pragma comment( lib , "marisad.lib" )
#else
#pragma comment( lib , "uppcore.lib" )
#pragma comment( lib , "uppctrlcore.lib" )
#pragma comment( lib , "uppctrllib.lib" )
#pragma comment( lib , "uppdraw.lib" )
#pragma comment( lib , "upprichtext.lib" )
#pragma comment( lib , "uppreport.lib" )
#pragma comment( lib , "upprichedit.lib" )
#pragma comment( lib , "zlib.lib" )
#pragma comment( lib , "marisa.lib" )
#endif
#include <Core/Core.h>
#include <Core/zip.h>

//using namespace Upp;
//void SearchForFiles( Vector<String>& files, String dir  )
//{
//    FindFile ff( AppendFileName( dir, "*.*" ) );
//    while( ff )
//    {
//        if( ff.IsFolder() && *ff.GetName() != '.' && *ff.GetName() != '..' )
//        {
//            files.Add( AppendFileName( dir, ff.GetName() ) );
//            SearchForFiles( files, AppendFileName( dir, ff.GetName() ) );
//        }
//        else if( ff.IsFile()  )
//        {
//            files.Add( AppendFileName( dir, ff.GetName() ) );
//        }
//        ff.Next();
//    }
//}
//int APIENTRY _tWinMain( HINSTANCE hInstance,
//                        HINSTANCE hPrevInstance,
//                        LPTSTR    lpCmdLine,
//                        int       nCmdShow )
//{
//    UNREFERENCED_PARAMETER( hPrevInstance );
//    UNREFERENCED_PARAMETER( lpCmdLine );
//
//    //Upp::FileStream f;
//    //f.Open( "d:\\uppzip.zip" , Upp::BlockStream::READWRITE | Upp::BlockStream::CREATE );
//    Vector<String> files;
//    SearchForFiles( files , String( "E:\\Test\\upp_vc\\obj" ) );
//
//    FileZip zip( "d:\\uppzip.zip" );
//    for( int i = 0 ; i < files.GetCount(); i++ )
//    {
//        String path = files[i];
//        path.Replace( String( "E:\\Test\\upp_vc\\obj\\" ) , String( "" ) );
//        if( DirectoryExists( files[i] ) )
//        {
//            zip.WriteFolder( path , GetUtcTime() );
//        }
//        else
//        {
//            FileMapping fm( ~files[i] , false );
//            fm.Map( 0, fm.GetFileSize() );
//            zip.WriteFile( fm.Begin() , fm.GetCount() , path , false , GetUtcTime() );
//        }
//    }
//    return 0;
//}


