= GASとAWSを使ってWeb操作をハックしてみた!

//flushright{
大野 泰歩
//}

== はじめに

はじめまして！ヤスと申します。

私は普段通信事業者として商用設備の運用保守業務を担当しております。運用保守業務を実施されたことのある方であれば経験があるかと思いますが、保守しているシステムの不具合や故障の対処を日々実施していく必要があります。この業務を滞りなく遂行していくには情報の扱いを出来る限り自動にしていく事が望ましいです。

私はGoogle Apps Script@<fn>{fn-0801}(GAS)やAmazon Web Services@<fn>{fn-0802}(AWS)を用いてWebページの操作を自動化の部分を実施しましたが、こちらをご説明する前に既存で作成されていた情報収集の自動化部分からも記載していこうかと思います。

GASやAWSなどの技術については独学で学んだモノになりますので、拙い部分も多々あるかと思いますが、あくまで自動化の一例として温かい目で拝読頂けると幸いです。

//footnote[fn-0801][https://developers.google.com/apps-script?hl=ja]
//footnote[fn-0802][https://docs.aws.amazon.com/ja_jp/]

== 情報収集の自動化(GAS)

「はじめに」で少し触れたように、私は運用保守業務を実施するうえで様々なインシデントを管理しており、こちらを管理するのにあたりスプレッドシートを用いております。毎度手動で入力していくのは大変なため、Gmailで通知されるインシデントから必要な情報を抜き出し、スプレッドシートに転記されるようにGASを用いて自動化しています。

自動化の方法としては通知されるインシデントメールのテンプレートを定義することでGASを用いて必要項目を抜き出しスプレッドシートに転記するというものになります。

メールの例文を2種類程紹介します。

・メールサンプル1

//emlist{
■ID
1111111

■ホストグループ名
service_ABC

■ホスト名
hostname_ABC

■アラームテキスト
・Error messeage
//}

・メールサンプル2

//emlist{
[Linuxtime] 1111111
[Hostname] hostname_ABC
[Serial] abc123
//}

ご紹介した様にテンプレートとしてメール本文の内容を決めることが出来た場合は、GASを用いてメール本文から情報を抜き出し、スプレッドシートに転記することが出来ます。

今回は正規表現を用いて情報を抜き出す際の例を記載いたします。

・メール本文の抜き出し(GAS)

//emlist{
var myThreads = GmailApp.search('メール件名', 0, 50);
var myMsgs = GmailApp.getMessagesForThreads(myThreads);

for ( var threadIndex = 0 ; threadIndex < myThreads.length ; threadIndex++ ) {
    var mailBody = myMsgs[threadIndex][0].getPlainBody();
//}

・メールサンプル1_抜き出し方法例(GAS)

//emlist{
var myRegexp = new RegExp('■ID' + '[\\s\\S]*?' + '■');
if (mailBody.match(myRegexp)) {
      var incidentID  = mailBody.match(myRegexp)[0].split('\n')[1];
}
else {
    continue;
}
//}

・メールサンプル2_抜き出し方法例(GAS)

//emlist{
var myRegexp = new RegExp('\\[Linuxtime\\] ' + '.*?' + '\r');
if (mailBody.match(myRegexp)) {
    var incidentID = mailBody.match(myRegexp)[0].replace('[Linuxtime]', '');
} 
else {
    incidentID = "Linuxtime未記載";
}
//}

このようにメール本文から情報を変数として取得することが出来ましたので、後はスプレッドシートの中で転記したい位置に取得した情報を記載するようにすれば情報の取得について自動化することが出来ました。

その他に補足情報が必要な場合は、他のスプレッドシートに記載している情報を同じ様にGASを用いたり関数を使用することで更に多くの情報を集めることもできます。

== Web操作の自動化(AWS)

情報の取得について自動にすることが出来たので、集めた情報を元にどの様にWeb操作を自動にしていくかを記載いたします。

元々はGASを用いてWeb操作を実施しようと思っておりましたが、GASのみでWeb操作を可能とするドキュメントを見つけることが出来なかったため、AWSを用いて自動化していく事にしました。

またこの後説明していく処理の流れがイメージしやすいように概要を添付いたします。

//image[08_001][処理概要]

添付した「処理概要」の通りになりますが、下記の流れで処理を行っていきます。

 1. スプレッドシートに情報収集
 2. GASからAPI GatewayにLambdaの呼び出し処理
 3. LambdaにてWeb画面の自動操作
 4. 処理結果をGASに返信

まずはLambdaでの処理を記載していきます。今回LambdaではPythonを使用していきます。Pythonを選択した理由としてはWeb画面の操作を実施するのに有名なSeleniumというモジュールを使用するためとなります。

自動化をするにあたって、このタイミングで1度躓きました。躓いた理由としてはLambdaの中には標準的な関数しか搭載されておらずSeleniumなどの拡張モジュールを使用することが出来なかったからとなります。そのため拡張モジュールを使用するためにLambdaの中のレイヤ―という機能として使用したいモジュールの追加を行いツールを実行出来るようにしました。

今回追加したモジュールは4つになります。

 * Chromedriver: Seleniumの処理を行う土台のサイトとして使用するため
 * headless-chromium: Web画面を表示しない状態で使用するため
 * Selenium: Web画面操作の自動化を行うため
 * headless-selenium: ヘッドレスモードでWeb画面操作を行うため

//image[08_002][レイヤ―情報]

ここまで準備出来たら後はpythonを用いてSeleniumの処理を記載することでWeb画面の操作が出来るようになります。

・import内容(Lambda)

//emlist{
import json
import time
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.common.keys import Keys
from selenium.webdriver.support.select import Select
//}

・API Gatewayと連携するための関数(Lambda)

//emlist{
def lambda_handler(event, context):
    # TODO implement
    body = json.loads(event['body'])
    sample_value = sample_function(body)
    
    return {
        'statusCode': 200,
        'body': json.dumps(sample_value)
    }
//}

・Chromeドライバの設定(Lambda)

//emlist{
def headless_chrome():
    ## Seleniumが使用するWebDriver作成時のオプションを作る
    options = webdriver.ChromeOptions()
    ## オプションのバイナリロケーションにLayerで用意したheadless-chromiumのパスを指定
    options.binary_location = '/opt/headless/bin/headless-chromium'
    ## オプションにヘッドレスモードで実行させる記述を書く
    options.add_argument("--headless")
    options.add_argument("--no-sandbox")
    options.add_argument("--single-process")
    options.add_argument("--disable-gpu")
    options.add_argument("--homedir=/tmp")

    driver = webdriver.Chrome(
        ## Layerで用意したchromedriverのパスを指定
        executable_path="/opt/headless/bin/chromedriver",
        options = options
    )
    return driver
//}

・Selenium使用例(Lambda)

//emlist{
def sample_function(body):
    driver = headless_chrome()
    driver.delete_all_cookies()
    driver.get('https://www.google.co.jp/')
    search_box = driver.find_element_by_xpath('//*[@id="APjFqb"]')
    search_box.send_keys(body['sample'])
//}

この4つの内容を記載することでプログラムの基本的な概要は完成しております。後は、実際に使用する場合に合わせて必要なオプションを追加したりSeleniumの処理を追記していく事で実行したい処理を完成させていく事が出来ます。

Seleniumの使い方や関数の詳細については色々と書籍やWebページでの説明があるので、今回の自動化をする際に良く使用していた関数を代表として紹介します。

 * find_element_by_xpath: Webページのxpathを指定して要素を取得する
 * send_keys: テキストボックスなどに文字列を入力する
 * select_by_value: ドロップダウンなどのセレクト要素を選択する
 * click: 選択している要素をクリックする

これにてWeb画面の操作を自動にする処理をLambdaを用いて作成することが出来ました。そのため後はAPI Gatewayを作成し、今回作ったLambdaと連携させることでGASから呼び出せるようになります。

== GASからLambdaの呼び出し(GAS)

GASからLambdaに連携するにあたってAPI Gatewayで生成した連携用のURLと収集した情報をjsonの形で整えて使用する必要があります。その整えた情報をUrlFetchApp.fetchを使用することでLambdaに渡すことが出来ます。

・GASからLambdaへの連携(GAS)

//emlist{
const aws_url = 'API GatewayのURL';
const params = {
  'method' : 'post', //get or post
  'contentType': 'application/json',
  'payload' : JSON.stringify(
    {"sample_value1" : sample_value1,
    "sample_value2" : sample_value2,
    "sample_value3" : sample_value3})
};

const req = UrlFetchApp.fetch(aws_url, params);
//}

これにてGASとLambdaの連携が完了しましたので、後は処理が完了した旨のポップアップを表示させたりメールを送るなどお好みで設定するかたちとなります。

== さいごに

本章においては、定常的な業務の効率化を行うための方法について記載させて頂きました。AWSの使い方やGASの使用方法について他にも最適な方法はあるかと思いますが、この章をお読みいただいた方の業務が少しでも楽になったり、効率化への閃きに寄与することがあれば幸いです。最後になりますが本章をお読みいただきましてありがとうございました。

