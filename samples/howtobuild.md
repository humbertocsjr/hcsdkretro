```sh
cd hello-8086
hcbuild test.prj make release

cd ..
cd calc-z80
hcbuild calc.prj make release

```

# Hello World with RetroLang (hello-rl-8086)

In Windows or customized environments, you must edit the hellorl.prj file and update the SDK addresses to match the installed SDK paths.


```sh
cd hello-rl-8086
hcbuild hellorl.prj make release

```