/****************************************************************************************************************************
  ESP32-S3 + W5500 Ethernet mDNS 테스트 프로젝트

  목적: ESP32에서 Ethernet 사용 시 mDNS가 작동하는지 다양한 방법으로 테스트
*****************************************************************************************************************************/

#include <M5Unified.h>
#include <SPI.h>
#include <Ethernet.h>
#include <WiFi.h>
#include <ESPmDNS.h>

#define SerialDebug Serial

// 핀 설정
#define USE_THIS_SS_PIN 9

// MAC 주소
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0x99};

// 테스트 변수
unsigned long lastMDNSCheck = 0;
const unsigned long MDNS_CHECK_INTERVAL = 5000; // 5초마다 체크

void printSeparator()
{
    SerialDebug.println("\n========================================");
}

void testMethod1_EthernetOnly()
{
    printSeparator();
    SerialDebug.println("🧪 테스트 1: Ethernet만 사용 (순수 Ethernet)");
    printSeparator();

    // Ethernet 초기화
    SerialDebug.printf("📡 Ethernet 초기화 (CS 핀: %d)\n", USE_THIS_SS_PIN);
    Ethernet.init(USE_THIS_SS_PIN);

    SerialDebug.println("📡 DHCP로 네트워크 연결 중...");
    if (Ethernet.begin(mac, 10000, 1000) == 0)
    {
        SerialDebug.println("❌ DHCP 실패!");
        return;
    }

    SerialDebug.println("✅ DHCP 성공!");
    SerialDebug.printf("🌐 IP 주소: %s\n", Ethernet.localIP().toString().c_str());

    // mDNS 초기화 시도 (Ethernet만)
    SerialDebug.println("\n📡 mDNS 초기화 시도 (Ethernet 인터페이스)...");
    if (MDNS.begin("m5stack-eth"))
    {
        SerialDebug.println("✅ mDNS 초기화 성공!");
        MDNS.addService("http", "tcp", 80);
        SerialDebug.println("✅ HTTP 서비스 등록 완료");
        SerialDebug.println("🏷️  호스트명: m5stack-eth.local");
    }
    else
    {
        SerialDebug.println("❌ mDNS 초기화 실패!");
    }

    // 10초간 mDNS 작동 테스트
    SerialDebug.println("\n⏱️  10초간 mDNS 작동 테스트...");
    for (int i = 0; i < 10; i++)
    {
        SerialDebug.printf("  %d초 경과...\n", i + 1);
        delay(1000);
    }

    SerialDebug.println("\n⚠️  테스트 1 완료. 다른 PC에서 ping m5stack-eth.local 테스트 해보세요.");
    delay(3000);
}

void testMethod2_WiFiStack()
{
    printSeparator();
    SerialDebug.println("🧪 테스트 2: WiFi 스택 활성화 후 Ethernet + mDNS");
    printSeparator();

    // Ethernet 초기화
    SerialDebug.printf("📡 Ethernet 초기화 (CS 핀: %d)\n", USE_THIS_SS_PIN);
    Ethernet.init(USE_THIS_SS_PIN);

    SerialDebug.println("📡 DHCP로 네트워크 연결 중...");
    if (Ethernet.begin(mac, 10000, 1000) == 0)
    {
        SerialDebug.println("❌ DHCP 실패!");
        return;
    }

    SerialDebug.println("✅ DHCP 성공!");
    SerialDebug.printf("🌐 Ethernet IP: %s\n", Ethernet.localIP().toString().c_str());

    // WiFi 스택 초기화 (연결 없이)
    SerialDebug.println("\n📶 WiFi 스택 활성화 (mDNS용)...");
    WiFi.mode(WIFI_STA);

    // WiFi IP를 Ethernet과 같은 서브넷으로 설정
    IPAddress ethIP = Ethernet.localIP();
    IPAddress wifiIP(ethIP[0], ethIP[1], ethIP[2], ethIP[3] + 1);
    IPAddress gateway = Ethernet.gatewayIP();
    IPAddress subnet = Ethernet.subnetMask();
    IPAddress dns = Ethernet.dnsServerIP();

    WiFi.config(wifiIP, gateway, subnet, dns);
    SerialDebug.printf("📶 WiFi IP 설정: %s (가상)\n", wifiIP.toString().c_str());

    // 더미 SSID로 연결 시도 (스택만 활성화)
    WiFi.begin("__mdns_test__", "__pass__");
    delay(3000);
    WiFi.disconnect(false); // 연결은 끊되 WiFi 모드 유지

    SerialDebug.printf("📶 WiFi 모드: %d, 상태: %d\n", WiFi.getMode(), WiFi.status());

    // mDNS 초기화 시도
    SerialDebug.println("\n📡 mDNS 초기화 시도 (WiFi 스택 활성화 상태)...");
    if (MDNS.begin("m5stack-eth2"))
    {
        SerialDebug.println("✅ mDNS 초기화 성공!");
        MDNS.addService("http", "tcp", 80);
        SerialDebug.println("✅ HTTP 서비스 등록 완료");
        SerialDebug.println("🏷️  호스트명: m5stack-eth2.local");
    }
    else
    {
        SerialDebug.println("❌ mDNS 초기화 실패!");
    }

    // 10초간 mDNS 작동 테스트
    SerialDebug.println("\n⏱️  10초간 mDNS 작동 테스트...");
    for (int i = 0; i < 10; i++)
    {
        SerialDebug.printf("  %d초 경과...\n", i + 1);
        delay(1000);
    }

    SerialDebug.println("\n⚠️  테스트 2 완료. 다른 PC에서 ping m5stack-eth2.local 테스트 해보세요.");
    delay(3000);
}

void testMethod3_EthernetFirst()
{
    printSeparator();
    SerialDebug.println("🧪 테스트 3: Ethernet 먼저, WiFi 나중에, mDNS 마지막");
    printSeparator();

    // 1. Ethernet 먼저 초기화
    SerialDebug.printf("📡 1단계: Ethernet 초기화 (CS 핀: %d)\n", USE_THIS_SS_PIN);
    Ethernet.init(USE_THIS_SS_PIN);

    SerialDebug.println("📡 DHCP로 네트워크 연결 중...");
    if (Ethernet.begin(mac, 10000, 1000) == 0)
    {
        SerialDebug.println("❌ DHCP 실패!");
        return;
    }

    SerialDebug.println("✅ DHCP 성공!");
    SerialDebug.printf("🌐 Ethernet IP: %s\n", Ethernet.localIP().toString().c_str());
    delay(2000);

    // 2. WiFi 스택 초기화
    SerialDebug.println("\n📶 2단계: WiFi 스택 활성화...");
    WiFi.mode(WIFI_AP_STA); // AP+STA 모드로 시도

    // AP 모드로 WiFi 스택 활성화
    SerialDebug.println("📶 WiFi AP 모드 시작...");
    WiFi.softAP("M5Stack-mDNS-Test", "");
    IPAddress apIP = WiFi.softAPIP();
    SerialDebug.printf("📶 AP IP: %s\n", apIP.toString().c_str());
    delay(2000);

    // 3. mDNS 초기화
    SerialDebug.println("\n📡 3단계: mDNS 초기화...");
    if (MDNS.begin("m5stack-eth3"))
    {
        SerialDebug.println("✅ mDNS 초기화 성공!");
        MDNS.addService("http", "tcp", 80);
        SerialDebug.println("✅ HTTP 서비스 등록 완료");
        SerialDebug.println("🏷️  호스트명: m5stack-eth3.local");
    }
    else
    {
        SerialDebug.println("❌ mDNS 초기화 실패!");
    }

    // 10초간 mDNS 작동 테스트
    SerialDebug.println("\n⏱️  10초간 mDNS 작동 테스트...");
    for (int i = 0; i < 10; i++)
    {
        SerialDebug.printf("  %d초 경과...\n", i + 1);
        delay(1000);
    }

    SerialDebug.println("\n⚠️  테스트 3 완료. 다른 PC에서 ping m5stack-eth3.local 테스트 해보세요.");

    // AP 모드 종료
    WiFi.softAPdisconnect(true);
    delay(3000);
}

void runAllTests()
{
    SerialDebug.println("\n\n");
    SerialDebug.println("╔════════════════════════════════════════╗");
    SerialDebug.println("║  ESP32 Ethernet + mDNS 테스트 시작    ║");
    SerialDebug.println("╚════════════════════════════════════════╝");
    SerialDebug.println();

    // 테스트 1: Ethernet만 사용
    testMethod1_EthernetOnly();
    MDNS.end();
    delay(2000);

    // 테스트 2: WiFi 스택 활성화
    testMethod2_WiFiStack();
    MDNS.end();
    WiFi.mode(WIFI_OFF);
    delay(2000);

    // 테스트 3: Ethernet → WiFi → mDNS 순서
    testMethod3_EthernetFirst();
    MDNS.end();
    WiFi.mode(WIFI_OFF);
    delay(2000);

    printSeparator();
    SerialDebug.println("🏁 모든 테스트 완료!");
    printSeparator();
    SerialDebug.println("\n📊 테스트 결과 요약:");
    SerialDebug.println("  - 각 테스트 중 다른 PC에서 ping 명령어로 확인");
    SerialDebug.println("  - 예: ping m5stack-eth.local");
    SerialDebug.println("  - 예: ping m5stack-eth2.local");
    SerialDebug.println("  - 예: ping m5stack-eth3.local");
    SerialDebug.println("\n⚠️  ESP32의 mDNS는 WiFi 인터페이스에 의존하므로");
    SerialDebug.println("    Ethernet만으로는 작동하지 않을 가능성이 높습니다.");
    SerialDebug.println();
}

void setup()
{
    // M5Stack 초기화
    auto cfg = M5.config();
    cfg.output_power = true;
    M5.begin(cfg);

    SerialDebug.begin(115200);
    while (!Serial && millis() < 5000)
        ;
    delay(1000);

    SerialDebug.println("\n\n=== M5Stack CoreS3 + Ethernet mDNS Test ===");
    SerialDebug.printf("보드: ESP32-S3\n");
    SerialDebug.printf("칩 리비전: %d\n", ESP.getChipRevision());
    SerialDebug.printf("플래시 크기: %d MB\n", ESP.getFlashChipSize() / 1024 / 1024);
    SerialDebug.println();

    // 모든 테스트 실행
    runAllTests();

    // 최종적으로 가장 가능성 높은 방법으로 설정 (테스트 3 방식)
    SerialDebug.println("\n🔄 최종 설정: 테스트 3 방식으로 지속 운영...\n");

    // Ethernet 초기화
    Ethernet.init(USE_THIS_SS_PIN);
    if (Ethernet.begin(mac, 10000, 1000) == 0)
    {
        SerialDebug.println("❌ 최종 Ethernet 설정 실패!");
        return;
    }
    SerialDebug.printf("✅ Ethernet 연결: %s\n", Ethernet.localIP().toString().c_str());

    // WiFi AP 모드
    WiFi.mode(WIFI_AP);
    WiFi.softAP("M5Stack-mDNS", "");
    SerialDebug.printf("✅ WiFi AP: %s\n", WiFi.softAPIP().toString().c_str());

    // mDNS 시작
    if (MDNS.begin("m5stack"))
    {
        SerialDebug.println("✅ mDNS 시작: m5stack.local");
        MDNS.addService("http", "tcp", 80);
    }
    else
    {
        SerialDebug.println("❌ mDNS 시작 실패");
    }

    SerialDebug.println("\n🌍 접속 가능한 주소:");
    SerialDebug.printf("  - http://%s (Ethernet IP)\n", Ethernet.localIP().toString().c_str());
    SerialDebug.printf("  - http://m5stack.local (mDNS, 작동 시)\n");
    SerialDebug.println();
}

void loop()
{
    M5.update();

    // 5초마다 상태 출력
    if (millis() - lastMDNSCheck > MDNS_CHECK_INTERVAL)
    {
        lastMDNSCheck = millis();

        SerialDebug.println("🔍 현재 상태:");
        SerialDebug.printf("  - Ethernet IP: %s\n", Ethernet.localIP().toString().c_str());
        SerialDebug.printf("  - WiFi AP IP: %s\n", WiFi.softAPIP().toString().c_str());
        SerialDebug.printf("  - WiFi 모드: %d\n", WiFi.getMode());
        SerialDebug.printf("  - 업타임: %lu초\n", millis() / 1000);
        SerialDebug.println("  - mDNS 호스트: m5stack.local");
        SerialDebug.println();
    }

    delay(10);
}
