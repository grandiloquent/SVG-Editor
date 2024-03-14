plugins {
    alias(libs.plugins.android.application)
}

android {
    namespace = "psycho.euphoria.svg"
    compileSdk = 34

    defaultConfig {
        applicationId = "psycho.euphoria.svg"
        minSdk = 24
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
}
