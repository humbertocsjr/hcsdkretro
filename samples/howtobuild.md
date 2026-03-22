```sh
cd hello-8086
hcbuild test.prj make release

cd ..
cd calc-z80
hcbuild calc.prj make release

```

# Hello World with RetroLang (hello)

In Windows or customized environments, you must edit the cpm-z80.prj/dos-8086.prj file and update the SDK addresses to match the installed SDK paths.


```sh
cd hello
hcbuild cpm-z80.prj make release
hcbuild dos-8086.prj make release

```