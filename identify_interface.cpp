#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
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

// read_atoms関数: 原子の位置とタイプを読み取り、2次元ベクターを返す
std::vector<std::vector<Atom>> read_atoms(SimulationInfo &si,
                                          const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "ファイルを開けませんでした。" << std::endl;
    return {}; // 空のベクターを返す
  }

  std::string line;
  bool atoms_found = false;
  bool box_found = false;
  std::vector<std::vector<Atom>> all_frames; // 全スナップショットの原子データ
  std::vector<Atom> current_frame; // 現在のスナップショットの原子データ

  while (std::getline(file, line)) {
    if (!box_found && line.find("ITEM: BOX BOUNDS") != std::string::npos) {
      // ボックスサイズを読み取る
      double x_min, x_max, y_min, y_max, z_min, z_max;
      std::getline(file, line);
      std::istringstream(line) >> x_min >> x_max;
      si.LX = x_max - x_min;

      std::getline(file, line);
      std::istringstream(line) >> y_min >> y_max;
      si.LY = y_max - y_min;

      std::getline(file, line);
      std::istringstream(line) >> z_min >> z_max;
      si.LZ = z_max - z_min;

      box_found = true;
    }

    if (!atoms_found &&
        line.find("ITEM: NUMBER OF ATOMS") != std::string::npos) {
      // 次の行に原子数がある
      std::getline(file, line);
      si.atoms = std::stoi(line);
      atoms_found = true;
    }

    if (line.find("ITEM: ATOMS id type xs ys zs") != std::string::npos) {
      // "ITEM: ATOMS" を見つけた時
      static int frame_count = 0;
      std::cout << "Frame " << frame_count++ << std::endl;

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

      all_frames.push_back(current_frame); // フレームを全体に追加
    }
  }

  file.close();

  // 2次元ベクターの返却
  return all_frames;
}

int main() {
  std::string filename = "rho0.6_640x80x10_T1.2_N1_A5_F0.03/"
                         "rho0.6_640x80x10_T1.2_N1_A5_F0.03.lammpstrj";
  SimulationInfo si = read_info(filename);

  std::cout << "原子数: " << si.atoms << std::endl;
  std::cout << "LX: " << si.LX << std::endl;
  std::cout << "LY: " << si.LY << std::endl;
  std::cout << "LZ: " << si.LZ << std::endl;

  // 次に原子データを読み取る（2次元ベクターとして返される）
  std::vector<std::vector<Atom>> all_frames = read_atoms(si, filename);

  return 0;
}
