plugins {
    alias(libs.plugins.android.application)
}
android {
    namespace = "psycho.euphoria.svg"
    compileSdk = 34

    defaultConfig {
        applicationId = "psycho.euphoria.svg"
        minSdk = 26
        targetSdk = 34
        versionCode = 1
        versionName = "1.0"
        ndk {
            abiFilters += listOf("arm64-v8a")
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_1_8
        targetCompatibility = JavaVersion.VERSION_1_8
    }
    externalNativeBuild {
        cmake {
            path = file("/src/main/jni/CMakeLists.txt")
        }
    }
    packagingOptions {
        exclude("META-INF/DEPENDENCIES")
        exclude("META-INF/LICENSE")
        exclude("META-INF/LICENSE.txt")
        exclude("META-INF/license.txt")
        exclude("META-INF/NOTICE")
        exclude("META-INF/NOTICE.txt")
        exclude("META-INF/notice.txt")
        exclude("META-INF/ASL2.0")
        exclude("META-INF/*.kotlin_module")
    }
}
dependencies { // All the libraries you want to use. See 3️⃣
    implementation(libs.pdfbox.android)
    // https://mvnrepository.com/artifact/com.gemalto.jp2/jp2-android
    //implementation(libs.jp2.android)





}