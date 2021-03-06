5号機（小型2号機）前段解析用パッケージ

　　　　　　　　　　　　　　　11-Dec-2017
                              藤井啓文(KEK)

これは、5号機（小型2号機）前段解析用の
プログラムパッケージです。
視野軸を真上に向けた観測を想定しています。

前段解析手順は、
1.Acceptance の計算
2.解析するファイル名のリスト生成
3.解析プログラムの実行
です。このパッケージには、これらを計算
するプログラムが含まれています。

解析では、例えば、射影面上での吸収画像を
得るには、
「3.解析プログラムの実行結果」
を 
「1.Acceptance の計算結果」
で割り算することで得ることができます。

-------------------
1.Acceptance の計算
-------------------
Program名：	projaccd5
機能：		射影面までの距離が有限である場合の acceptance を
		計算します。出力は csv 形式で行われます。
呼び出し形式：	projaccd5 [options]
options:
	-dist value	射影面までの距離（高さ）を m 単位で与えます。
			原点は、下の XY ユニットの X 面と Y面の中央です。
	-dzshift value	XY ユニット間の（高さ方向の）距離を mm 単位で与えます。
	-out name	出力ファイルにつける前置名を与えます。
			出力ファイルは、この名前に -0.csv、-1.csv、.. を
			付けて複数出力されます。

Histogram の出力は
 　   *-0.csv, *-1.csv  組み合わせの数および weight smoothing を適用したもの
      *-2.csv, *-3.csv  acceptance および weight smoothing を適用したもの
      *-4.csv, *-5.csv  acceptance に cos^2 を掛けたものおよび weight smoothing を
                        適用したもの

このプログラムは、射影面の位置、XY ユニット間距離等、
幾何学的な設定を変えなければ一度計算するだけで、その都度
計算する必要はありません。

--------------------------------
2.解析するファイル名のリスト生成
--------------------------------
Program名：	gencoinlistKEK
機能：		開始 run 番号、終了 run 番号を与えると、解析するファイル名の
		リストを生成します。解析プログラムは、
		このリストに記載されたファイル名を順に解析していきます。
		リストは、テキストファイルで、1行に1ファイル名が記載
		されます。
		解析プログラムでは、このリストを先頭行から順に読んで
		解析し、ヒストグラムを生成します。

呼び出し形式：	gencoinlistKEK start_run_no end_run_no
	start_run_no	開始 run number
	end_run_no	終了 run number

解析するデータファイルは kekirid.kek.jp の /data1/Detector5/data 以下に
保存されているものとしています。
標準出力に出力されるので、ファイルに保存するには re-direct してください。


------
3.解析
------
