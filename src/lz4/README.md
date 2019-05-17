LZ4 - Extremely fast compression
================================

LZ4 is lossless compression algorithm,
providing compression speed > 500 MB/s per core,
scalable with multi-cores CPU.
It features an extremely fast decoder,
with speed in multiple GB/s per core,
typically reaching RAM speed limits on multi-core systems.

Speed can be tuned dynamically, selecting an "acceleration" factor
which trades compression ratio for faster speed.
On the other end, a high compression derivative, LZ4_HC, is also provided,
trading CPU time for improved compression ratio.
All versions feature the same decompression speed.

LZ4 is also compatible with [dictionary compression](https:
and can ingest any input file as dictionary,
including those created by [Zstandard Dictionary Builder](https:
(note: only the final 64KB are used).

LZ4 library is provided as open-source software using BSD 2-Clause license.


|Branch      |Status   |
|------------|---------|
|master      | [![Build Status][travisMasterBadge]][travisLink] [![Build status][AppveyorMasterBadge]][AppveyorLink] [![coverity][coverBadge]][coverlink] |
|dev         | [![Build Status][travisDevBadge]][travisLink]    [![Build status][AppveyorDevBadge]][AppveyorLink]                                         |

[travisMasterBadge]: https:
[travisDevBadge]: https:
[travisLink]: https:
[AppveyorMasterBadge]: https:
[AppveyorDevBadge]: https:
[AppveyorLink]: https:
[coverBadge]: https:
[coverlink]: https:

> **Branch Policy:**
> - The "master" branch is considered stable, at all times.
> - The "dev" branch is the one where all contributions must be merged
    before being promoted to master.
>   + If you plan to propose a patch, please commit into the "dev" branch,
      or its own feature branch.
      Direct commit to "master" are not permitted.

Benchmarks
-------------------------

The benchmark uses [lzbench], from @inikep
compiled with GCC v7.3.0 on Linux 64-bits (Debian 4.15.17-1).
The reference system uses a Core i7-6700K CPU @ 4.0GHz.
Benchmark evaluates the compression of reference [Silesia Corpus]
in single-thread mode.

[lzbench]: https:
[Silesia Corpus]: http:

|  Compressor             | Ratio   | Compression | Decompression |
|  ----------             | -----   | ----------- | ------------- |
|  memcpy                 |  1.000  |13100 MB/s   |  13100 MB/s   |
|**LZ4 default (v1.8.2)** |**2.101**|**730 MB/s** | **3900 MB/s** |
|  LZO 2.09               |  2.108  |  630 MB/s   |    800 MB/s   |
|  QuickLZ 1.5.0          |  2.238  |  530 MB/s   |    720 MB/s   |
|  Snappy 1.1.4           |  2.091  |  525 MB/s   |   1750 MB/s   |
|  [Zstandard] 1.3.4 -1   |  2.877  |  470 MB/s   |   1380 MB/s   |
|  LZF v3.6               |  2.073  |  380 MB/s   |    840 MB/s   |
| [zlib] deflate 1.2.11 -1|  2.730  |  100 MB/s   |    380 MB/s   |
|**LZ4 HC -9 (v1.8.2)**   |**2.721**|   40 MB/s   | **3920 MB/s** |
| [zlib] deflate 1.2.11 -6|  3.099  |   34 MB/s   |    410 MB/s   |

[zlib]: http:
[Zstandard]: http:

LZ4 is also compatible and optimized for x32 mode,
for which it provides additional speed performance.


Installation
-------------------------

```
make
make install     # this command may require root permissions
```

LZ4's `Makefile` supports standard [Makefile conventions],
including [staged installs], [redirection], or [command redefinition].
It is compatible with parallel builds (`-j#`).

[Makefile conventions]: https:
[staged installs]: https:
[redirection]: https:
[command redefinition]: https:


Documentation
-------------------------

The raw LZ4 block compression format is detailed within [lz4_Block_format].

Arbitrarily long files or data streams are compressed using multiple blocks,
for streaming requirements. These blocks are organized into a frame,
defined into [lz4_Frame_format].
Interoperable versions of LZ4 must also respect the frame format.

[lz4_Block_format]: doc/lz4_Block_format.md
[lz4_Frame_format]: doc/lz4_Frame_format.md


Other source versions
-------------------------

Beyond the C reference source,
many contributors have created versions of lz4 in multiple languages
(Java, C#, Python, Perl, Ruby, etc.).
A list of known source ports is maintained on the [LZ4 Homepage].

[LZ4 Homepage]: http:
