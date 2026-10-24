# CarGallery

> 컴퓨터그래픽스 기말프로젝트 · Interactive 3D Virtual Exhibition

가상 공간에서 열리는 **모빌리티쇼**입니다. 사용자는 전시장을 자유롭게 걸어 다니며 여러 대의 차량을 둘러보고, 마음에 드는 차량 앞에 다가가 **직접 커스터마이징**할 수 있습니다. 실제 모터쇼에서는 할 수 없는 "내 취향대로 바꿔 보기"를 가상 전시의 핵심 경험으로 삼았습니다.

## 주요 기능

- **전시장 탐색**: 1인칭 시점으로 벽·바닥·천장이 있는 실내 전시장을 이동
- **차량 전시**: 5대 이상의 차량을 전시대 위에 배치하고 조명으로 연출
- **차량 상호작용**: 차량 근처로 다가가면 상호작용 모드로 진입
- **커스터마이징**: 차량의 외관(색상, 재질 등)을 실시간으로 변경

## 조작법 (예정)


| 입력              | 동작                |
| --------------- | ----------------- |
| `W` `A` `S` `D` | 이동                |
| 마우스             | 시점 회전             |
| `E`             | 차량과 상호작용          |
| `Esc`           | 상호작용 종료 / 프로그램 종료 |


## 기술 스택

- C++, OpenGL 3.3 Core Profile (GLSL 셰이더)
- GLFW (창·입력), GLAD (OpenGL 로더), GLM (수학), Dear ImGui (UI)

## 팀 구성 및 역할


| 담당  | 내용                                          |
| --- | ------------------------------------------- |
| 왕민  | 카메라 이동, 전시장 모델링, 조명, 차량 배치, 차량 근처 상호작용 진입 |
| 손혜성 | 차량 에셋 선정, 커스터마이징 UI, 차량 외관 변경               |


## 일정


| 시기           | 내용                                                        |
| ------------ | --------------------------------------------------------- |
| ~ 10/25      | 각자 맡은 기능 구현 (손혜성: 차량 에셋 선정 및 커스텀 요소 구현/ 왕민: 공간 및 상호작용 구현) |
| 10주차 (11/04) | 프로젝트 개요서 제출 , 1차 발표 (중간 점검)                               |


## 외부 에셋 출처


| 에셋        | 출처  | 라이선스 |
| --------- | --- | ---- |
| Lamborghini Countach 2021 (`countach.obj`) | ["Free Lamborghini Countach 2021"](https://sketchfab.com/3d-models/free-lamborghini-countach-2021-cf4e16b74ef7499abbdf591c665fc0ee) by Desiccated_Lemon | CC BY 4.0 |
| Tesla Roadster 2020 (`roadster.obj`) | ["Tesla Roadster 2020"](https://sketchfab.com/3d-models/tesla-roadster-2020-1fbf29e297bd4a17ac39a00a378441d8) by metarex.4d | CC BY 4.0 |
| Mercedes-Benz S65 W222 (`s65.obj`) | ["Mersedes-Benz s65 w222"](https://sketchfab.com/3d-models/mersedes-benz-s65-w222-f42d9d13b0404b5b9a2302a4c590a94d) by Black Snow | CC BY 4.0 |
| Jeep Gladiator 2020 (`gladiator.obj`) | ["2020 Jeep Gladiator"](https://sketchfab.com/3d-models/2020-jeep-gladiator-98bc399af1e948eda5db6d75c5bb649d) by Ddiaz Design | CC BY-NC-SA 4.0 |
| Kia Optima K5 (`optima.obj`) | ["Kia optima k5"](https://sketchfab.com/3d-models/kia-optima-k5-6fc788c08348419d92588f2f541c5b50) by dannzjs | CC BY 4.0 |

차량은 Sketchfab glTF를 `tools/gltf2obj.py`로 OBJ로 바꾼 것입니다. 모양(정점·법선·면)과 재질 이름만 남기고 색·텍스처는 버렸으며, 외관은 코드에서 직접 정합니다. 원본 라이선스 문구는 `CarGallery/assets/cars/*_license.txt`에 있습니다.


## 빌드

IDE 프로젝트 파일은 각자 관리합니다. `CarGallery/*.cpp`, `external/src/glad.c`, `external/imgui/*.cpp`를 컴파일하고, include 경로에 `external/include`와 `external/imgui`를 추가한 뒤 GLFW를 링크하세요. 실행 작업 디렉터리는 `CarGallery/`입니다.