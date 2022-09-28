#include <catch2/catch_all.hpp>

#include "../functions.hpp"
#include <iostream>


bool runBash( const std::string &command ) {
    std::string cmd = "/bin/bash -c \"" + command + "\"";
    return system( cmd.c_str() ) == 0;
}


TEST_CASE( "iterateFS" ) {
    const std::string directory = "test_dir";
    runBash(
        "mkdir " + directory + " ; cd test_dir ; for f in {01..10} ; do "
        "mkdir \\\"Season \\${f}\\\" ; cd \\\"Season \\${f}\\\" ; for g in "
        "{01..30} ; do touch \\\"S\\${f}E\\${g}.mkv\\\" ; done ; cd .. ; "
        "done" );
    auto files = getFilesInSource("test_dir");
    REQUIRE( files.size() == 310 );
    REQUIRE(files[0].getFileType() == FileType::TYPE_DIRECTORY);
    REQUIRE(files[0].getDepth() == 0);
    REQUIRE_THAT(files[0].getPath(), Catch::Matchers::Equals("Season 01") || Catch::Matchers::Equals("Season 1"));
    REQUIRE(files[1].getFileType() == FileType::TYPE_FILE);
    REQUIRE(files[1].getDepth() == 1);
    REQUIRE_THAT(files[1].getPath(), Catch::Matchers::EndsWith(".mkv"));

    REQUIRE_THROWS_WITH( getFilesInSource( "nonexistendir" ),
                         Catch::Matchers::ContainsSubstring( "Directory" ) &&
                             Catch::Matchers::ContainsSubstring( "doesn't exist" ) );
    REQUIRE_THROWS_WITH(
        getFilesInSource( directory + "/Season 01/S01E01.mkv" ),
        Catch::Matchers::ContainsSubstring( "Directory" ) &&
            Catch::Matchers::ContainsSubstring( "isn't a directory" ) );
    runBash("rm -Rf " + directory );
}
