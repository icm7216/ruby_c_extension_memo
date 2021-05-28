# RubyのC拡張メモ

## 概要

これは、Windowsユーザー向けのRubyのC拡張ライブラリの作り方のメモです。「RubyのC拡張ライブラリは、どうやって作るんだろう？」という好奇心駆動学習によって、調べて学んだことを簡単にまとめてみました。作業は Windows 10で行っています。
C拡張ライブラリのコンパイルからgem生成までを書いていますが、Ruby C APIについては（まだ勉強途中なので）詳しく触れていません。

#### Windowsの環境変数について:

Windowsのコマンドプロンプトで拡張ライブラリのコンパイルを行う場合、環境変数にRubyの開発環境のパスを追加してDevkitのコマンド（make, gcc 等）を使えるようにしなければなりません。

[RubyInstaller for Windows](https://rubyinstaller.org/)でインストールしたRuby環境には、そのためのコマンドツールが用意されています。その中の`ridk enable`コマンドは、環境変数にDevkitのパスを追加してくれる便利なコマンドです。ridkコマンドツールについて詳しく知りたいときは [The ridk tool](https://github.com/oneclick/rubyinstaller2/wiki/The-ridk-tool) を読んでみてください。


### 私の環境 (2021/05/27): 
*  Microsoft Windows [Version 10.0.19043.1023] 21H1
*  ruby 2.7.3p183 (2021-04-05 revision 6847ee089d) [x64-mingw32]
   => [rubyinstaller: Ruby+Devkit 2.7.3-1 (x64)](https://rubyinstaller.org/downloads/)
*  Bundler version 2.2.17

### 参考

Ruby C APIやRubyGemsなど：

*  [extension.ja.rdoc - (ruby-doc)](https://ruby-doc.org/core-3.0.1/doc/extension_ja_rdoc.html)
*  [関数一覧 - (Ruby リファレンスマニュアル)](https://docs.ruby-lang.org/ja/2.7.0/function/index.html)
*  [Rubyソースコード完全解説](https://i.loveruby.net/ja/rhg/book/)

*  [Gems with Extensions - (RubyGems Guides)](https://guides.rubygems.org/gems-with-extensions/)
*  [library rubygems - (Ruby リファレンスマニュアル)](https://docs.ruby-lang.org/ja/2.7.0/library/rubygems.html)
*  [rake-compiler](https://github.com/rake-compiler/rake-compiler)
*  [The ridk tool - (oneclick/rubyinstaller2 Wiki)](https://github.com/oneclick/rubyinstaller2/wiki/The-ridk-tool)
*  [bundle gem - (Bundler)](https://bundler.io/v2.2/man/bundle-gem.1.html)

書籍：
*  [プログラミングRuby 第2版 言語編 - Ohmsha](https://www.ohmsha.co.jp/book/9784274066429/)
*  [パーフェクトRuby - 技術評論社](https://gihyo.jp/book/2013/978-4-7741-5879-2)
*  [オブジェクト指向スクリプト言語 Ruby - アスキー](https://www.amazon.co.jp/%E3%82%AA%E3%83%96%E3%82%B8%E3%82%A7%E3%82%AF%E3%83%88%E6%8C%87%E5%90%91%E3%82%B9%E3%82%AF%E3%83%AA%E3%83%97%E3%83%88%E8%A8%80%E8%AA%9E-ASCII-SOFTWARE-SCIENCE-Language/dp/4756132545/)

---

# 拡張ライブラリ

RubyのライブラリにはRubyで記述されたライブラリや、CやC++で記述されたライブラリが有ります。特に、CやC++で記述されたライブラリを拡張ライブラリと呼んでいます。これらのライブラリは、Rubyを再コンパイルすること無く、動的リンクの仕組みを使って実行時に組み込むことができます。また、RubyとCのやり取りが容易にできるように、RubyのC APIが用意されています。

# Step1: 拡張ライブラリの基本構成
  
はじめに、拡張ライブラリを小さな構成で試してみましょう。
例えば`hello`と言う名前の拡張ライブラリを作る場合、最小構成では次のようなファイルがあれば作成できます。

```
>tree
.
├── extconf.rb
└── hello.c
```

## extconf.rb

extconf.rbはMakefileを生成するための設定ファイルです。mkmfライブラリを使用して拡張ライブラリのコンパイルに必要な外部ヘッダやライブラリのパスを追加したり、存在チェックなどを行うコードをこのファイルに書いたりします。

Makefileの生成には、`create_makefile`メソッドを使います。メソッドの引数`hello`は、ターゲットの`hello.c`に相当します。また、requireで指定するライブラリ名にもなります。

`extconf.rb`
```ruby
require "mkmf"

create_makefile("hello")
```

## hello.c

`hello.c`はC拡張ライブラリの本体です。ファイル名は「ライブラリ名.c」を使います。ここでは、拡張ライブラリ名`hello`なので`hello.c`を使用します。

ここでは簡単なサンプルとして、rubyで書くと次のようなコードをCで作成してみましょう。

```ruby
module Hello
  class Hello
    def say
      "Hello Ruby"
    end
  end
end
```

`hello.c`

*  `#include "ruby.h"`は、Ruby C APIを使うための記述です。
*  hello_say は、Cで記述したsayメソッドの本体です。このメソッドは、Rubyの文字列`"Hello Ruby"`を返します。
*  Init_hello は、拡張ライブラリhelloを最初に読み込んだ時に実行される初期化関数です。ここではライブラリの初期化に必要な、モジュール、クラス、メソッド、定数の定義などを行います。
   *  rb_define_module("Hello")は、`Hello`モジュールを作成。
   *  rb_define_class_under(mHello, "Hello", rb_cObject)は、`Hello`モジュールの中に　`Hello`クラスを作成。
   *  rb_define_method(cHello, "say", hello_say, 0)は、Rubyの`say`メソッドとCの関数`hello_say`をバインドしています。

```c
#include "ruby.h"

static VALUE
hello_say(VALUE self)
{
  return rb_str_new_cstr("Hello Ruby");
}

void
Init_hello(void)
{
  VALUE mHello;  
  VALUE cHello;

  mHello = rb_define_module("Hello");
  cHello = rb_define_class_under(mHello, "Hello", rb_cObject);
  rb_define_method(cHello, "say", hello_say, 0);
}

```

## Makefileを生成。

```
>ruby extconf.rb
creating Makefile
```

## make

```
>make           
generating hello-x64-mingw32.def
compiling hello.c
linking shared-object hello/hello.so
```

コンパイルに成功すると共有ライブラリ(hello.so)が生成されます。このファイルがRubyのC拡張ライブラリです。
```
>tree
.
├── extconf.rb
├── hello.c
├── hello.o
├── hello.so
├── hello-x64-mingw32.def
└── Makefile
```

このとき、Devkitにパスが通っていないと下記のようなエラーでmakeに失敗します。予め`ridk enable`コマンドを実行して、忘れずにDevkitにパスを通しておきましょう。
```
>make
generating hello-x64-mingw32.def
make: *** No rule to make target `/C/Ruby27-x64/include/ruby-2.7.0/ruby.h', needed by `hello.o'.  Stop.
```

## 拡張ライブラリの動作確認

先ほど生成した拡張ライブラリを試しに実行してみましょう。拡張ライブラリを使うには`hello.so`をrequireするだけです。requireするときに、拡張子`.so`は付けても付けなくても大丈夫です。Rubyが適切な拡張子を補完してファイルを見つけてくれます。
```ruby
>irb
irb(main):001:0> require "./hello"    
=> true
irb(main):002:0> hello = Hello::Hello.new
=> #<Hello::Hello:0x000001d7ebf89868>
irb(main):003:0> hello.say           
=> "Hello Ruby"
irb(main):004:0> quit
```
うまく動きました。


今回の例のように、ソースファイル`hello.c`が一つだけでファイル名とライブラリ名が同じ。かつ、ヘッダやライブラリのチェックの必要が無く、extconf.rbの中身が`create_makefile("hello")`だけの場合は、あえてextconf.rbを作らずにワンライナーでも可能です。

この場合、拡張ライブラリに必要なファイルは`hello.c`だけで十分なので、つぎのワンライナーでMakefileを生成できます。
```
>ruby -r mkmf -e 'create_makefile("hello")'
```

---

# Step2: Rakeを使う

Step1では全てのファイルが一つのディレクトリの中に入っていました。
Step2ではgemのパッケージレイアウトに合わせて、ディレクトリを分けてファイルを整理しましょう。いくつかのディレクトリの追加と、rake-compilerを使うためのRakefileを作成します。そして、ビルド関連の操作をRakeコマンドで行えるようにします。

```
>tree
.
├── ext
│   └── hello
│       ├── extconf.rb
│       └── hello.c
├── lib
└── Rakefile
```

*  [rake-compiler](https://github.com/rake-compiler/rake-compiler)は、RubyのC拡張ライブラリのビルドやパッケージ化をサポートしてくれる便利なRakeタスクです。
インストールがまだの場合はここでインストールしておきましょう。
```
>gem install rake-compiler
```

*  Rakeは、Rubyで書かれたビルドツールです。
   [library rake (Ruby 2.7.0 リファレンスマニュアル)](https://docs.ruby-lang.org/ja/2.7.0/library/rake.html)


*  gem(RubyGems)は、ライブラリを管理するための便利な仕組みです。[gemコマンド](https://docs.ruby-lang.org/ja/2.7.0/library/rubygems.html)を使ってライブラリのインストール、アンインストール、更新や作成等を行います。


## extconf.rb

extconf.rbの内容はStep1と同じです。`ext`ディレクトリに、拡張ライブラリ名のサブディレクトリ`hello`を作成してファイルを配置します。

`ext/hello/extconf.rb`
```ruby
require "mkmf"

create_makefile("hello")
```



## hello.c

hello.cの内容もStep1と同じ内容です。`ext`ディレクトリに、拡張ライブラリ名のサブディレクトリ`hello`を作成してファイルを配置します。

`ext/hello/hello.c`
```c
#include "ruby.h"

static VALUE
hello_say(VALUE self)
{
  return rb_str_new_cstr("Hello Ruby");
}

void
Init_hello(void)
{
  VALUE mHello;  
  VALUE cHello;

  mHello = rb_define_module("Hello");
  cHello = rb_define_class_under(mHello, "Hello", rb_cObject);
  rb_define_method(cHello, "say", hello_say, 0);
}

```


## Rakefile

Rakefileを作成して、rake-compilerを使うためのRake::ExtensionTaskを追加します。このとき`lib_dir`で、拡張ライブラリの配置場所を指定できます。この例では`lib/hello`に拡張ライブラリ`hello.so`を配置します。

`Rakefile`
```ruby
require "rake/extensiontask"

Rake::ExtensionTask.new("hello") do |ext|
  ext.lib_dir = "lib/hello"
end
```

ここでRakeタスクを確認しておきましょう。
一覧にrake-compilerの cleanタスクや clobberタスク、compileタスクが追加されたことがわかります。これでRakeコマンドを使ってビルド関連の操作ができるようになりました。
```ruby
>rake -T
rake clean          # Remove any temporary products
rake clobber        # Remove any generated files
rake compile        # Compile all the extensions
rake compile:hello  # Compile hello
```


## 拡張ライブラリをコンパイル

（ RubyInstaller2のRubyユーザーは、コンパイルの前に`ridk enable`コマンドを実行してDevkitのパスを追加しておきましょう。 ）

Rakeタスクのcompileを実行して、拡張ライブラリ(hello.so)を生成します。
```
>rake compile
mkdir -p tmp/x64-mingw32/hello/2.7.3
cd tmp/x64-mingw32/hello/2.7.3
C:/Ruby27-x64/bin/ruby.exe -I. ../../../../ext/hello/extconf.rb
creating Makefile
cd -
cd tmp/x64-mingw32/hello/2.7.3       
C:\Ruby27-x64\msys64\usr\bin/make.exe
generating hello-x64-mingw32.def
compiling ../../../../ext/hello/hello.c
linking shared-object hello.so
cd -
mkdir -p tmp/x64-mingw32/stage/lib/hello
install -c tmp/x64-mingw32/hello/2.7.3/hello.so lib/hello/hello.so
cp tmp/x64-mingw32/hello/2.7.3/hello.so tmp/x64-mingw32/stage/lib/hello/hello.so  
```

Rake::ExtensionTaskの指定どおり`lib/hello/hello.so`に拡張ライブラリが配置されました。

```
>tree
.
├── ext
│   └── hello
│       ├── extconf.rb
│       └── hello.c   
├── lib
│   └── hello
│       └── hello.so  
├── Rakefile
└── tmp
    └── x64-mingw32   
        ├── hello     
        │   └── 2.7.3 
        │       ├── hello.o
        │       ├── hello.so
        │       ├── hello-x64-mingw32.def
        │       └── Makefile
        └── stage
            └── lib
                └── hello
                    └── hello.so
```


## 拡張ライブラリの動作確認

irbで動作確認してみましょう。
ここでは未だgemを作成していないので、拡張ライブラリのパス`./lib/hello/hello`を直接指定して読み込みます。

```ruby
>irb
irb(main):001:0> require "./lib/hello/hello"
=> true
irb(main):002:0> hello = Hello::Hello.new   
=> #<Hello::Hello:0x0000020f3f7a1160>
irb(main):003:0> hello.say                  
=> "Hello Ruby"
irb(main):004:0> quit 
>
```

うまく動きました。


---

# Step3: gemを生成

Rakefileを修正して、gemを生成してみましょう。（ここでもextconf.rbと hello.cの内容はStep2と同じです。）

```
>tree -A 
.
├── ext
│   └── hello
│       ├── extconf.rb
│       └── hello.c
├── lib
└── Rakefile
```


## Rakefile

Step2のRakefileに`Gem::PackageTask`を記述して、新たにパッケージタスクを追加しましょう。Gem::Specificationには、gemを作成するための情報を適宜記述しておきます。


`Rakefile`
```ruby
require "rake/extensiontask"

Rake::ExtensionTask.new("hello") do |ext|
  ext.lib_dir = "lib/hello"
end

require 'rake/packagetask'

spec = Gem::Specification.new do |s|
  s.name        = "hello"
  s.version     = "0.0.0"
  s.summary     = "This is Hello example!"
  s.authors     = ["Hello author"]
  s.homepage    = "https://rubygems.org/gems/example"
  s.licenses    = ["MIT"]
  s.files       = FileList["{lib,ext}/**/*", "Rakefile"]
  s.extensions  = FileList["ext/**/extconf.rb"]
end

Gem::PackageTask.new(spec) do |pkg|
end

```

ここでRakeタスクを確認しておきましょう。
Rakeタスクを確認すると、一覧には gemタスクや、packageタスクなどが追加されたことがわかります。
```ruby
>rake -T
rake clean            # Remove any temporary products
rake clobber          # Remove any generated files
rake clobber_package  # Remove package products
rake compile          # Compile all the extensions
rake compile:hello    # Compile hello
rake gem              # Build the gem file hello-0.0.0.gem
rake package          # Build all the packages
rake repackage        # Force a rebuild of the package files

```

## gemを生成

rake gemコマンドでgemを生成します。

```
>rake gem
mkdir -p pkg
mkdir -p pkg/hello-0.0.0
rm -f pkg/hello-0.0.0/Rakefile      
ln Rakefile pkg/hello-0.0.0/Rakefile
mkdir -p pkg/hello-0.0.0/ext
mkdir -p pkg/hello-0.0.0/ext/hello
rm -f pkg/hello-0.0.0/ext/hello/extconf.rb
ln ext/hello/extconf.rb pkg/hello-0.0.0/ext/hello/extconf.rb
rm -f pkg/hello-0.0.0/ext/hello/hello.c
ln ext/hello/hello.c pkg/hello-0.0.0/ext/hello/hello.c
cd pkg/hello-0.0.0
  Successfully built RubyGem
  Name: hello
  Version: 0.0.0
  File: hello-0.0.0.gem
cd -
```

`pkg`ディレクトリの中に`hello-0.0.0.gem`が生成されました。

```
>tree
.
├── ext
│   └── hello
│       ├── extconf.rb
│       └── hello.c
├── lib
├── pkg
│   ├── hello-0.0.0
│   │   ├── ext
│   │   │   └── hello
│   │   │       ├── extconf.rb
│   │   │       └── hello.c
│   │   └── Rakefile
│   └── hello-0.0.0.gem
└── Rakefile
```

ディレクトリツリーの中に`hello.so`が存在しませんが、gemのインストール時にコンパイルを行って自動的に生成されます。

それではgemをインストールして動作確認しましょう。

```
>gem install pkg/hello-0.0.0.gem
Building native extensions. This could take a while...
Successfully installed hello-0.0.0
Parsing documentation for hello-0.0.0
Installing ri documentation for hello-0.0.0
Done installing documentation for hello after 0 seconds
1 gem installed
```

## 拡張ライブラリの動作確認

インストールした拡張ライブラリ`hello`を読み込むには`require "hello"`を使います。

```ruby
>irb
irb(main):001:0> require "hello"
=> true
irb(main):002:0> hello = Hello::Hello.new
=> #<Hello::Hello:0x000001f9591bbf58>
irb(main):003:0> hello.say
=> "Hello Ruby"
irb(main):004:0> quit
```

うまく動きました。

---

# Step4: Bundlerを使う

[Bundler](https://bundler.io/)はアプリケーションで使用するgemパッケージを管理するツールです。

Bundlerはgemパッケージの雛型を生成する[bundle gem](https://bundler.io/v2.2/man/bundle-gem.1.html)コマンドを持っています。このコマンドを使うと、必要なファイルがそろったgemの雛型を生成できます。

C拡張を含む雛型を生成する場合は、コマンドオプションに`--ext`を指定します。ここではテストフレームワークに`test-unit`を指定しています。

```
bundle gem --ext --test=test-unit hello
```

実際に`hello`gemの雛型を生成してみましょう。
```
>bundle gem --ext --test=test-unit hello
Creating gem 'hello'...
test-unit is already configured, ignoring --test flag.
MIT License enabled in config
Initializing git repo in C:/temp/ruby_c_extension_memo/step_4/hello
      create  hello/Gemfile
      create  hello/lib/hello.rb
      create  hello/lib/hello/version.rb
      create  hello/hello.gemspec
      create  hello/Rakefile
      create  hello/README.md
      create  hello/bin/console
      create  hello/bin/setup
      create  hello/.gitignore
      create  hello/test/test_helper.rb
      create  hello/test/hello_test.rb
      create  hello/LICENSE.txt
      create  hello/ext/hello/extconf.rb
      create  hello/ext/hello/hello.h
      create  hello/ext/hello/hello.c

...snip...

Gem 'hello' was successfully created. For more information on making a RubyGem visit https://bundler.io/guides/creating_gem.html
```

雛型を作成後、C拡張とテストを書いていきます。

## hello.c

ここでは、`hello.h`の中で`ruby.h`をインクルードしているので、`hello.c`では`hello.h`をインクルードしています。その他の内容はStep3と同じです。

`hello/ext/hello/hello.c`
```c
#include "hello.h"

static VALUE
hello_say(VALUE self)
{
  return rb_str_new_cstr("Hello Ruby");
}

void
Init_hello(void)
{
  VALUE mHello;  
  VALUE cHello;

  mHello = rb_define_module("Hello");
  cHello = rb_define_class_under(mHello, "Hello", rb_cObject);
  rb_define_method(cHello, "say", hello_say, 0);
}

```


## hello_test.rb

C拡張のsayメソッドのテストも書いておきましょう。
`hello/test/hello_test.rb`
```
require "test_helper"

class HelloTest < Test::Unit::TestCase
  test "VERSION" do
    assert do
      ::Hello.const_defined?(:VERSION)
    end
  end

  test "say Hello" do
    hello = Hello::Hello.new
    actual = hello.say
    expected = "Hello Ruby"
    assert_equal(expected, actual)
  end
end
```

Rakeタスクの一覧を表示すると、新たにinstall`やrelease、testなどが増えています。
```
>cd hello
>rake -T
rake build            # Build hello-0.1.0.gem into the pkg directory
rake build:checksum   # Generate SHA512 checksum if hello-0.1.0.gem into th...
rake clean            # Remove any temporary products
rake clobber          # Remove any generated files
rake compile          # Compile all the extensions
rake compile:hello    # Compile hello
rake install          # Build and install hello-0.1.0.gem into system gems
rake install:local    # Build and install hello-0.1.0.gem into system gems ...
rake release[remote]  # Create tag v0.1.0 and build and push hello-0.1.0.ge...
rake test             # Run tests
```

## コンパイルとテスト

生成されたRakefileのデフォルトタスクには、clobberとcompile、testが記述されています。
このようにデフォルトタスクを書いておくと、コンパイルに成功すると続いてテストが実行されます。

```
task default: %i[clobber compile test]
```

Rakeのデフォルトタスクを実行
```
>rake
mkdir -p tmp/x64-mingw32/hello/2.7.3
cd tmp/x64-mingw32/hello/2.7.3
C:/Ruby27-x64/bin/ruby.exe -I. ../../../../ext/hello/extconf.rb
creating Makefile
cd -
cd tmp/x64-mingw32/hello/2.7.3
C:\Ruby27-x64\msys64\usr\bin/make.exe
generating hello-x64-mingw32.def
compiling ../../../../ext/hello/hello.c
linking shared-object hello/hello.so
cd -
mkdir -p tmp/x64-mingw32/stage/lib/hello
install -c tmp/x64-mingw32/hello/2.7.3/hello.so lib/hello/hello.so
cp tmp/x64-mingw32/hello/2.7.3/hello.so tmp/x64-mingw32/stage/lib/hello/hello.so
Loaded suite C:/Ruby27-x64/lib/ruby/gems/2.7.0/gems/rake-13.0.1/lib/rake/rake_test_loader    
Started
..
Finished in 0.0054253 seconds.
---------------------------------------------------------------------------------------------
2 tests, 2 assertions, 0 failures, 0 errors, 0 pendings, 0 omissions, 0 notifications        
100% passed
---------------------------------------------------------------------------------------------
368.64 tests/s, 368.64 assertions/s
```

コンパイルとテストが完了しました。
これまで毎回手作業の動作確認を行っていましたが、代わりにテストを書いておけば全て自動で確認できるので便利ですね。


## hello.gemspec

gemを生成する前に、gemの情報をhello.gemspecに記述しておきましょう。

`hello/hello.gemspec`
```ruby
# frozen_string_literal: true

require_relative "lib/hello/version"

Gem::Specification.new do |spec|
  spec.name          = "hello"
  spec.version       = Hello::VERSION
  spec.authors       = ["Hello author"]
  spec.email         = ["hello_author@example.com"]

  spec.summary       = "hello summary"
  spec.description   = "hello description'"
  spec.homepage      = "http://example.com/hello/"
  spec.license       = "MIT"
  spec.required_ruby_version = Gem::Requirement.new(">= 2.4.0")


  # Specify which files should be added to the gem when it is released.
  # The `git ls-files -z` loads the files in the RubyGem that have been added into git.
  spec.files = Dir.chdir(File.expand_path(__dir__)) do
    `git ls-files -z`.split("\x0").reject { |f| f.match(%r{\A(?:test|spec|features)/}) }
  end
  spec.bindir        = "exe"
  spec.executables   = spec.files.grep(%r{\Aexe/}) { |f| File.basename(f) }
  spec.require_paths = ["lib"]
  spec.extensions    = ["ext/hello/extconf.rb"]

  # Uncomment to register a new dependency of your gem
  # spec.add_dependency "example-gem", "~> 1.0"

  # For more information and examples about making a new gem, checkout our
  # guide at: https://bundler.io/guides/creating_gem.html
end
```

gemを生成する前に、もう一つ忘れてはいけないことがあります。

`hello/hello.gemspec`のファイルの一覧(spec.filesの部分)は次のように書かれています。
```ruby
# The `git ls-files -z` loads the files in the RubyGem that have been added into git.
spec.files = Dir.chdir(File.expand_path(__dir__)) do
  `git ls-files -z`.split("\x0").reject { |f| f.match(%r{\A(?:test|spec|features)/}) }
end
```
bunldle gemで生成した雛型では、gemパッケージに含むファイルを`git ls-files`コマンドによって取得しています。これは、「gitで管理していないファイルはgemパッケージに含まないよ」と言うことなので、gemを生成する前に、これらのファイルのコミットを忘れないように気を付けましょう。

>この部分は、これまでgitを使ったことが無いWindowsユーザーが、初めてRubyを学習する時に嵌るところかも知れません。


準備ができたら`rake install`コマンドで、gemの生成とインストールを行います。

```
>rake install
install -c tmp/x64-mingw32/hello/2.7.3/hello.so lib/hello/hello.so
cp tmp/x64-mingw32/hello/2.7.3/hello.so tmp/x64-mingw32/stage/lib/hello/hello.so
hello 0.1.0 built to pkg/hello-0.1.0.gem.
hello (0.1.0) installed.

```

## 拡張ライブラリの動作確認

Step3と同様に、インストールした拡張ライブラリ`hello`を読み込むには`require "hello"`を使います。

```ruby
>irb
irb(main):001:0> require "hello"
=> true
irb(main):002:0> hello = Hello::Hello.new
=> #<Hello::Hello:0x0000028c662b9680>
irb(main):003:0> hello.say
=> "Hello Ruby"
irb(main):004:0> quit
```

うまく動きました。



---

# appendix


## test-unitの使い方。

本家のマニュアル
*   [test-unit - Ruby用単体テストフレームワーク](http://test-unit.github.io/ja/index.html)

使い方のドキュメント

*   [Ruby用単体テストフレームワークtest-unitでのデータ駆動テストの紹介 - ククログ(2013-01-23)](https://www.clear-code.com/blog/2013/1/23.html)
*   [Test::Unitでテストを書く - Qiita](https://qiita.com/repeatedly/items/727b08599d87af7fa671)
*   [Ruby 2.6.0とtest-unitとデータ駆動テスト - ククログ(2018-12-26)](https://www.clear-code.com/blog/2018/12/26.html)




## VSCodeのIntelliSense の構成

VSCodeのintelliSenseで`ruby.h`を認識させる場合は`c_cpp_properties.json`を作成しておくと便利です。

*   `#include "ruby.h"`の行をクリックして、左上の電球アイコンをクリック。
*   「"includePath"設定の編集」を選択。
*   "includePath"に`C:/Ruby27-x64/include/**`を追加。

```
0 <== エディタ上では、このあたりに電球アイコンが表示されているはず
#include "ruby.h"
~~~~~~~~~~~~~~~~~
```



`.vscode/c_cpp_properties.json`
includePathにruby.hのパスを設定。
```
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "${workspaceFolder}/**",
                "C:/Ruby27-x64/include/**"
            ],
        }
    ],
    "version": 4
}
```
