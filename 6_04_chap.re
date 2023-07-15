= スマートマットをハックしてさらに便利にする

//flushright{
北崎 恵凡
//}

== スマートマットとは

あらかじめ登録した商品の重さを定期的に計測し、指定した条件になると自動で発注するスマートデバイスです。
製品としてスマートマットライト@<fn>{fn-0401}とスマートマットクラウド@<fn>{fn-0402}の2種類があります。
スマートマットライトはAmazonの日用品(水やペーパータオルなど、約3,000の対象商品)の自動注文に特化しており、管理画面が用意されています。マットの価格@<fn>{fn-0403}は安価(セール時〜通常: 1,980〜2,980円)で、マット10枚まで利用可能です。
スマートマットクラウドは法人向けサービスで、棚卸自動化、入出庫管理、自動注文など、フルスペック・フルサービスの在庫管理・発注プラットフォームです。
違いの詳細は製品のヘルプセンター@<fn>{fn-0404}に掲載されています。

//image[04_001][戸棚のスマートマットライト]
//image[04_002][ペーパータオルの在庫]

//footnote[fn-0401][https://service.lite.smartmat.io/]
//footnote[fn-0402][https://www.smartmat.io/]
//footnote[fn-0403][https://www.amazon.co.jp/dp/B08G8B2928]
//footnote[fn-0404][https://smartshopping.my.site.com/help/s/article/000001605]

== スマートマットライト

スマートマットライトはマット本体を購入すれば月額利用料は発生しません(買い切り)。マットは単三電池4本で動作し、Wi-Fiへ接続します。定期的に(デフォルトでは1日4回)重さを測定し、AWSへデータが送信されます。(通信をキャプチャーして確認)
データは専用の管理画面@<fn>{fn-0405}から、対象のマットを選択→詳細情報→消費レポートへ移動して確認することができます。

//image[04_003][スマートマットライトの管理画面]
//image[04_004][詳細情報]
//image[04_005][消費レポート]

//footnote[fn-0405][https://lite.smartmat.io/]

== スマートマットライト1と2の違い

結論から書くと、機能上の違いはほとんどありません。
スマートマットライトの裏側のシール(図4の矢印)を剥がすとネジが1本現れ、プラスドライバーで外すことができます。側面はガラス板とプラスチックの境界(図5の矢印)にマイナスドライバーを差し込んで開けることができます。スマートマットライト1はESP-WROOM-02(ESP8266チップ)基板のWi-Fiアンテナを使用していますが、スマートマットライト2はWi-Fiアンテナ(図6の矢印)が外出しとなり、ネットワークの接続性が向上しています。

//image[04_006][スマートマットライトの裏側]
//image[04_007][スマートマットライトの側面]
//image[04_008][分解した中身(バージョン1)]
//image[04_009][分解した中身(バージョン2)]

== スマートマットライトをハックする

スマートマットライトをそのまま使っても、Amazonアカウントと連携して自動注文してくれたり、EメールやLINE(ID連携が必要)で通知されるので非常に便利ですが、注文先を変更したり、消費データだけを利用(分析)したり、他の機能と連携させたい場合があります。
これらのニーズを実現するためスマートマットクラウドはAPI連携@<fn>{fn-0406}が可能ですが、スマートマットライトはAPIが提供されていません。
そこでこの記事では、Google SpreadsheetとGAS(Google Apps Script)を使用して管理画面の消費レポートからデータを取得し、他の機能と連携させる方法を説明します。

//footnote[fn-0406][https://smartshopping.my.site.com/help/s/article/000001143]

== パケットキャプチャで通信の内容を確認

まず、macOSのインターネット共有の機能を利用して、スマートマットライトの通信内容をWireshark@<fn>{fn-0407}でパケットキャプチャします。

//footnote[fn-0407][https://www.wireshark.org/]

//image[04_010][接続構成]

すると、以下の処理をしていることが確認できます。

 1. DNSでネームを検索
 2. HTTP POSTで測定データを送信
 3. HTTP GETでデータを取得

//image[04_011][パケットキャプチャ]

== ブラウザの開発者ツールで通信の内容を確認

ブラウザの開発者ツールを使用して、スマートマットライトの管理画面にログインします。
すると、以下の処理をしていることが確認できます。

 1. サインインのURL(/api/bff/v1/signin)でID(email)とパスワード(password)を送信
 2. クッキー(cookie)でセッションID(sid)を受信

//image[04_012][ブラウザの開発者ツール]

== APIの仕様を推察

管理画面にログインした後、スマートマットのデータを確認します。
詳細情報を確認すると、スマートマット毎にURLに管理番号(subscriptionId)が付与されています。

//image[04_013][スマートマットの詳細情報]

HTTP GETでデータ取得用のURL(/api/bff/v1/subscription/search_detail)にアクセスし、以下のJSONデータを取得しています。

//emlist{
{
  "deviceSerialNumber": "WXXXXXXXXXXX",
  "isFirstConnected": false,
  "isConnected": true,
  "battery": 66,
  "remainingPercent": 0,
  "triggerRemainingPercent": 20,
  "measuredAt": "2023-04-08T06:06:03+09:00",
  "current": 0,
  "frequency": 8,
  "subscriptionId": 8594,
  "title": "Ar ハンドタオル 200枚入 5個パック",
  "full": 2070.21,
  "productUrl": "https://www.amazon.co.jp/dp/B00K0Z0ZFU",
  "imageUrl": "https://m.media-amazon.com/images/I/414WdyYdVuL._SL500_.jpg",
  "outputType": 4,
  "orderable": false,
  "dailyAverageConsumption": 6.5,
  "dailyConsumption": 0,
  "weeklyConsumption": 0,
  "totalOrderCount": 4
}
//}

これで、スマートマットクラウドのAPIから取得できるデータとフォーマットが一部共通であることを確認できました。

//table[04_001][データフォーマット]{
戻り値	説明	具体例
-------------
deviceSerialNumber	シリアル番号	W42190800XXX
isFirstConnected	(不明)	.
isConnected	Wi-Fi接続状況	true
battery	電池残量	73
remainingPercent	最新計測時刻の残量 %表示の場合のみ	57
triggerRemainingPercent	閾値(％表示の場合)	20
measuredAt	最新計測時刻	.
current	最新計測値(単位はグラム)	11330
frequency	計測頻度(1日n回)	1
subscriptionId	(不明)	.
title	商品名	お米
full	満タン重量 %表示の場合のみ	20000
productUrl	(不明)	.
imageUrl	商品画像URL	.
outputType	発注通知方法	メール通知
orderable	(不明)	.
dailyAverageConsumption	(不明)	.
dailyConsumption	(不明)	.
weeklyConsumption	(不明)	.
totalOrderCount	(不明)	.
//}

== GASの実装

通信の確認とAPIの解析結果をもとに、GAS(Google Apps Script)を作成します。
作成するスクリプトは大きく2つの処理から成ります。

 1. スマートマットライトの管理画面へログインし、スマートマットの測定データを取得する
 2. 取得したデータをスプレッドシートに記録する

GASの中の変数に必要な情報を設定します。

 * 「email」　→管理画面のログインID
 * 「password」　→管理画面のパスワード
 * 「XXXX」　→スマートマットの管理番号
 * 「sheet_id」　→スプレッドシートID
 * 「sheet_name」　→スプレッドシートのシート名

GitHubにサンプルコード@<fn>{fn-0408}を載せましたので参考にしてみてください。

//footnote[fn-0408][https://github.com/kitazaki/smartmat/blob/main/smartmat.gs]

//emlist{
function fetchJSONData() {

  const postdata = {
	'email': '', // ID
	'password': '' // Password
  }

  const loginUrl = 'https://lite.smartmat.io/api/bff/v1/signin'; // ログインURL
  const dataUrl = 'https://lite.smartmat.io/api/bff/v1/subscription/search_detail?subscriptionId=XXXX'; // データURL
  const sheet_id = '';  // SpreadSheetID
  const sheet_name = ''; // SpreadSheetName

  const options = {
	'method': 'post',
	'Content-Type': 'application/json',
	'payload': JSON.stringify(postdata),
	'followRedirects': false
  }

  // ログインページにPOSTリクエストを送信して、Cookieを取得する
  const response = UrlFetchApp.fetch(loginUrl, options);
  const headers = response.getHeaders();
  const cookie = headers['Set-Cookie'];

  // Cookieを使用して、JSONデータを取得する
  const jsonData = UrlFetchApp.fetch(dataUrl, {
	'headers': {
  	'Cookie': cookie
	}
  }).getContentText();

  // JSONデータを解析する
  const parsedData = JSON.parse(jsonData);

  // スマートマットがWi-Fiに接続されている場合: true → 1、切断されている場合: false → 0
  let connect = 0;
  if (parsedData['isConnected']) {
	connect = 1;
  }
 
  // JSONデータをSpreadSheetに出力する
  const MySheet = SpreadsheetApp.openById(sheet_id);
  MySheet.getSheetByName(sheet_name).appendRow(
	[new Date(), connect, parsedData['battery'], parsedData['remainingPercent']]
  );
}
//}

GASを保存したら「実行」を押します。

//image[04_014][GASの実行]

初めて実行する場合、承認が必要ですので、「権限を確認」を押します。

//image[04_015][権限を確認]

次に、アカウントを選択します。

//image[04_016][アカウントの選択]

「詳細」を押したあと、「（安全ではないページ）に移動」を選択します。

//image[04_017][詳細の選択]
//image[04_018][（安全ではないページ）に移動の選択]

外部サービスへの接続で「許可」を押します。

//image[04_019][外部サービスへの接続]

GASの実行が正常に完了すると、スプレッドシートにスマートマットライトのデータが記録されるので、記録されたデータを用いてグラフを作成します。

//image[04_020][スプレッドシート]
//image[04_021][グラフ]

GASを定期実行する場合、トリガーを設定します。時計マークを押します。

//image[04_022][トリガーの設定]

「トリガーを追加」を押します。

//image[04_023][トリガーを追加]

 * 「イベントのソースを選択」　→時間主導型
 * 「時間ベースのトリガーのタイプを選択」　→時間ベースのタイマー
 * 「時間の間隔を選択（時間）」　→1時間おき

を選択し、「保存」を押します。

//image[04_024][トリガーを保存]

== 結果表示

在庫の消費傾向に加えて、Wi-Fi接続状況と電池の消費傾向も把握できるようにしました。これで在庫が無くなる時期を予測したり、使い過ぎを警告することもできそうです。

//image[04_025][Wi-Fi接続状況・バッテリー容量・ストック残量のグラフ]

== さいごに

スプレッドシートとGASを使用して、スマートマットライトをハックしてみました。
みなさんもぜひ、いろんなIoTデバイスをハックして自動化を楽しんでみましょう！
