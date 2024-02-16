# SigFinder

[![Build status](https://ci.appveyor.com/api/projects/status/pem6dwj7lfpuxc37?svg=true)](https://ci.appveyor.com/project/hasherezade/sig-finder)

Allows to find binary signatures with wildcards in a given buffer. Example of a valid signature:

`40 ?? 4? 8? e?`

The list of signatures can be supplied as a file in SIG format:

```
<Signature name>
<Number of elements in the signature: N>
<N byte definitions>
```

Example:
```
ASProtect v1.1 MTEc
9
90 60 E8 1B ?? ?? ?? E9 FC
```
See also: [SIG.txt](https://github.com/hasherezade/pe-bear/blob/main/SIG.txt)
