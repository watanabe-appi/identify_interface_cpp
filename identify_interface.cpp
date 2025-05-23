#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <vector>

struct Atom {
  int type;
  double x, y, z;
};

struct SimulationInfo {
  int atoms;         // 原子数
  double LX, LY, LZ; // シミュレーションボックスのサイズ
};

SimulationInfo read_info(const std::string filename) {
  std::ifstream file(filename);
  SimulationInfo info;

  if (!file.is_open()) {
    std::cerr << "ファイルを開けませんでした: " << filename << std::endl;
    return info; // 初期値を返す
  }

  std::string line;
  bool atoms_found = false;
  bool box_found = false;

  while (std::getline(file, line)) {
    if (!atoms_found &&
        line.find("ITEM: NUMBER OF ATOMS") != std::string::npos) {
      // 次の行に原子数がある
      std::getline(file, line);
      info.atoms = std::stoi(line);
      atoms_found = true;
    }

    if (!box_found && line.find("ITEM: BOX BOUNDS") != std::string::npos) {
      // 次の3行でボックスサイズ情報
      double x_min, x_max, y_min, y_max, z_min, z_max;
      std::getline(file, line);
      std::istringstream(line) >> x_min >> x_max;
      info.LX = x_max - x_min; // X方向のサイズ

      std::getline(file, line);
      std::istringstream(line) >> y_min >> y_max;
      info.LY = y_max - y_min; // Y方向のサイズ

      std::getline(file, line);
      std::istringstream(line) >> z_min >> z_max;
      info.LZ = z_max - z_min; // Z方向のサイズ

      box_found = true;
    }

    // 必要な情報を取得したら処理を終了
    if (atoms_found && box_found) {
      break;
    }
  }

  file.close();
  return info;
}

// identify_interface関数:
// 系をx方向にM分割し、原子タイプ1のy座標の重心を計算してファイルに保存
void identify_interface(SimulationInfo &si, std::vector<Atom> &atoms,
                        const int M) {
  // static変数で呼ばれた回数をカウント
  static int count = 0;
  // M分割におけるビン幅
  double bin_width = si.LX / M;

  // ファイル名を"frame" + インデックス + ".dat"の形式にする
  std::ostringstream filename;
  filename << "interface_" << std::setw(3) << std::setfill('0') << count++
           << ".dat";
  std::cout << filename.str() << std::endl;

  // ファイルに出力
  std::ofstream outFile(filename.str());

  // ファイルが開けなかった場合はエラーメッセージを表示
  if (!outFile.is_open()) {
    std::cerr << "ファイルを開けませんでした: " << filename.str() << std::endl;
    return;
  }

  // 各ビンに含まれる原子タイプ1のy座標を格納するためのベクター
  std::vector<std::vector<double>> bin_y_coordinates(M);

  // 各原子について確認
  for (const auto &atom : atoms) {
    // 原子のx座標に対応するビン番号を計算
    int bin_idx = static_cast<int>(atom.x / bin_width);

    // ビン番号がM以上にならないように調整（最右のビン）
    if (bin_idx >= M)
      bin_idx = M - 1;

    // 原子タイプ1ならそのy座標を対応するビンに追加
    if (atom.type == 2) {
      bin_y_coordinates[bin_idx].push_back(atom.y);
    }
  }

  // ビンごとに重心を計算してファイルに出力
  for (int bin_idx = 0; bin_idx < M; ++bin_idx) {
    // ビンのx座標範囲
    double bin_min = bin_idx * bin_width;
    double bin_max = (bin_idx + 1) * bin_width;

    // ビン内に原子タイプ1の原子がいる場合、重心を計算
    if (!bin_y_coordinates[bin_idx].empty()) {
      // 重心の計算: y座標の平均
      double total_y = std::accumulate(bin_y_coordinates[bin_idx].begin(),
                                       bin_y_coordinates[bin_idx].end(), 0.0);
      double center_y = total_y / bin_y_coordinates[bin_idx].size();
      double bin_center_x = (bin_min + bin_max) / 2; // ビンのx座標の中心

      // ファイルにx, y座標を出力
      outFile << std::fixed << std::setprecision(2) << bin_center_x << " "
              << center_y << std::endl;
    }
  }

  // ファイルを閉じる
  outFile.close();
}

void read_atoms(const std::string &filename, const int M) {

  SimulationInfo si;
  // ファイルを読み込んでSimulationInfoとatomsを設定
  si = read_info(filename); // read_infoはシミュレーションの基本情報を読み取る
  std::cout << "Number of atoms: " << si.atoms << std::endl;
  std::cout << "System Size:" << std::endl;
  std::cout << "LX: " << si.LX << std::endl;
  std::cout << "LY: " << si.LY << std::endl;
  std::cout << "LZ: " << si.LZ << std::endl;

  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "ファイルを開けませんでした。" << std::endl;
  }

  std::string line;
  std::vector<Atom> current_frame; // 現在のスナップショットの原子データ

  while (std::getline(file, line)) {
    if (line.find("ITEM: ATOMS id type xs ys zs") != std::string::npos) {
      // "ITEM: ATOMS" を見つけた時
      static int frame_count = 0;
      frame_count++;

      // 原子座標を読み取る
      current_frame.clear(); // 新しいフレームの開始
      for (int i = 0; i < si.atoms; ++i) {
        std::getline(file, line);
        std::istringstream iss(line);

        int id, type;
        double xs, ys, zs;
        iss >> id >> type >> xs >> ys >> zs;

        // 座標を補正（周期境界条件）
        double x = fmod(xs * si.LX, si.LX);
        double y = fmod(ys * si.LY, si.LY);
        double z = fmod(zs * si.LZ, si.LZ);

        // x, y, zが負なら、シミュレーションボックスサイズを足す
        if (x < 0)
          x += si.LX;
        if (y < 0)
          y += si.LY;
        if (z < 0)
          z += si.LZ;

        // Atom構造体に格納
        current_frame.push_back({type, x, y, z});
      }
      identify_interface(si, current_frame, M);
    }
  }

  file.close();
}

// ファイルが存在するかどうかを確認する関数
bool file_exists(const std::string &filename) {
  struct stat buffer;
  return (stat(filename.c_str(), &buffer) == 0);
}

int main(int argc, char *argv[]) {
  // 実行時引数がない場合
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
    return 1;
  }

  std::string filename = argv[1];

  // ファイルが存在するか確認
  if (!file_exists(filename)) {
    std::cerr << "Error: File \"" << filename << "\" does not exist."
              << std::endl;
    return 1;
  }

  const int M = 256; // 分割数
  read_atoms(filename, M);
  return 0;
}
