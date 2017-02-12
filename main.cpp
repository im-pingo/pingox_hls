#include <cstdio>
#include "pgx_hls_helper.h"

int main()
{
	CHLSHelper hls(3/*segment num*/,
		true/*allow cache or not*/,
		3/*hls version*/,
		0x24/*h265*/, 0);
	hls.ResetStreamCache("/data/pgx_hls_root/", "subdir/", "pgx_test", 10);
	//hls.VideoMux(,);
    return 0;
}