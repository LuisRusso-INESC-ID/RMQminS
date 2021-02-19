# RMQminS 0.1.0-alpha

Implementation of algorithms for computing range minimum queries in minimal
space. This repository contains the implementation used for the
experimental evaluation proposed in the "Range Minimum Queries in Minimal
Space" [paper]. The main goal of these prototypes is to compare the
performance of the respective algorithms. Hence they are provided with a
simple CLI interface. The baseline prototype can be obtained from [Solon]'s
[rmqo] repository.

## Table of contents

- [Getting Started]
   - [Prerequisites]
   - [Installing]
   - [Running]
- [Contributing]
- [Versioning]
- [Authors]
- [License]
- [Acknowledgments]

## Getting Started

To get a copy of this software download or clone the GitHub repository.

Download:

```
wget https://github.com/LuisRusso-INESC-ID/RMQminS/archive/main.zip
```

Clone:

```
git clone https://github.com/LuisRusso-INESC-ID/RMQminS.git
```

### Prerequisites

This package was tested with [Gentoo], it will most likely work with other
Linux distributions and UNIX variants. Some version of the following
components must exist in the system.

For Linux:

* C compiler, [gcc] or [clang]
* [GNU Make]
* glibc

### Installing

Execute `make` to obtain the binaries `P`, `V` and `T2`.

```
make
```

### Running

First you need to create a file of commands in binary format. Use the `P`
binary that converts a file containing commands in string format into the
binary format. The string format is the one described in the [paper], where
commands and indicated by capital letters and commands and arguments are
separated by spaces, it is also possible to use newlines to separate
commands. The `input` file shows an example of this input, the example used
in the paper. To convert this file to binary format used the following
command:

```
./P input > bIn
```

The argument to the `P` binary is the name of the file that contains the
commands. This program outputs to `stdout` in binary, hence it is best to
redirect it as in the example.

It is now possible to use the file `bIn` to test the binaries `V` and
`T2`. Both binaries simply read their input from `stdin` hence they can be
used as follows:

```
./V < bIn
./T2 < bIn
```

Either of these commands should produce the following output:

```
4 6 27
3 6 26
```

This corresponds to the output of the `Q` and `C` commands. The output `4 6
27` is the output of the `Q 4` command. The first value is just the
repetition of the input argument `4`. The second value is the current
position in the array `A` in this case `6`. The third value is the solution
to the `Q` command. The solution to `Q 4` is `27`. In this case of the
close command there is no need to output a solution, but it is used for
debugging purposes. The solution to `C 3` is the same as `Q 3`, which is
`26`.

## Contributing

If you found this project useful please share it, also you can create an
[issue] with comments and suggestions.

## Versioning

We use [SemVer] for versioning. For the versions available, see the [tags]
on this repository.

## Authors

* **Luís M. S. Russo** - *Initial work* - [LuisRusso]

See also the list of [contributors] who participated in this project.

## License

This project is licensed under the MIT License - see
the [LICENSE file] for details

## Acknowledgments

* This software was produced for research that was funded in by national funds
through Fundação para a Ciência e Tecnologia ([FCT]) with reference
UIDB/50021/2020 and project NGPHYLO PTDC/CCI-BIO/29676/2017.

* Thanks to [PurpleBooth] for the [README-Template].
* The [grip] tool by [Joe Esposito] was very handy for producing this file.


[Getting Started]: #getting-started
[Prerequisites]: #prerequisites
[Installing]: #installing
[Running]: #running
[Contributing]: #contributing
[Versioning]: #versioning
[Authors]: #authors
[License]: #license
[Acknowledgments]: #acknowledgments

[paper]: https://arxiv.org/abs/2102.09463
[Solon]: https://homepages.cwi.nl/~solon/
[rmqo]: https://github.com/solonas13/rmqo
[Gentoo]: https://www.gentoo.org/
[gcc]: https://gcc.gnu.org/
[clang]: https://clang.llvm.org/
[GNU Make]: https://www.gnu.org/software/make/

[issue]: ../../issues
[lmsrusso@gmail.com]: mailto:lmsrusso@gmail.com
[SemVer]: http://semver.org/
[tags]: ../../tags
[LuisRusso]: https://github.com/LuisRusso-INESC-ID
[contributors]: ../../contributors
[LICENSE file]: ./LICENSE
[FCT]: https://www.fct.pt/
[PurpleBooth]: https://gist.github.com/PurpleBooth
[README-Template]: https://gist.github.com/PurpleBooth/109311bb0361f32d87a2
[grip]: https://github.com/joeyespo/grip
[Joe Esposito]: https://github.com/joeyespo
