


# qmake時の注意

qmake -project
したときは、myapp.proの最後に

    QT += core gui widgets serialport

    # ビルド生成物の出力先をソースフォルダ外に分離する設定
    OBJECTS_DIR = tmp/obj
    MOC_DIR = tmp/moc
    RCC_DIR = tmp/qrc

を加える　
