# 界面検出コード

## 使い方

まず`make`する。

```sh
make
```

すると、`identify_interface`という実行可能ファイルが作成されるので、

```sh
./identify_interface rho0.6_640x80x10_T1.2_N1_A5_F0.03/rho0.6_640x80x10_T1.2_N1_A5_F0.03.lammpstrj
```

のようにlammpstrjファイルを指定すれば界面検出をして`interface_XXX.dat`を出力する。出力先はカレントディレクトリであることに注意。

