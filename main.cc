#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include "src/decode.hh"
#include "src/display.hh"
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include <flags.hh>
// #include "src/bob.hh"

// Generate all pgm files an return the framerate of the file
void generate_pgm_files(std::string video_filename, std::vector<uint> &flags, std::vector<float> &frame_periods)
{
    // Remove the content of the pgm folder
    std::filesystem::remove_all("pgm");
    std::filesystem::create_directories("pgm");

    std::string command = "cd pgm ; ./../tools/mpeg2dec/src/mpeg2dec ../";
    command.append(video_filename);
    command.append(" -o pgm > infos.txt");
    int err = system(command.c_str());
    if (err == -1)
    {
        std::cerr << "Error: can't execute mpeg2dec command: " << command.c_str() << std::endl;
        exit(1);
    }

    std::fstream info_file;
    info_file.open("pgm/infos.txt");
    if (!info_file.is_open())
    {
        fprintf(stderr, "Error: can't open file %s", "pgm/infos.txt");
        exit(1);
    }

    // Get flags and frame periods
    std::string line;
    float new_frame_period;
    while (std::getline(info_file, line))
    {
        if (sscanf(line.c_str(), "Frame Period: %f", &new_frame_period) != 0)
        {
            frame_periods.push_back(new_frame_period);
            flags.push_back(CHANGE_PERIOD);
        }
        else
        {
            if (line.compare("FLAG_TOP_FIELD_FIRST"))
                flags.push_back(FLAG_TOP_FIELD_FIRST);
            else if (line.compare("FLAG_PROGRESSIVE_FRAME"))
                flags.push_back(FLAG_PROGRESSIVE_FRAME);
            else if (line.compare("FLAG_REPEAT_FIRST_FIELD"))
                flags.push_back(FLAG_REPEAT_FIRST_FIELD);
        }
    }

    info_file.close();
}

int main(int argc, char **argv)
{
    int opt;

    std::string video_filename;
    float framerate = -1.0f;
    bool on_screen = true;

    // Parse command-line arguments using getopt
    while ((opt = getopt(argc, argv, "v:f:p")) != -1)
    {
        switch (opt)
        {
        case 'v':
            video_filename = optarg;
            break;
        case 'f':
            framerate = std::stod(optarg);
            break;
        case 'p':
            on_screen = false;
            break;
        default:
            std::cerr << "Usage: " << argv[0] << " [-v filename] [-f framerate] [-p]" << std::endl;
            return 1;
        }
    }

    // *** Generate all pgm files *** //
    std::vector<uint> flags = std::vector<uint>();
    std::vector<float> frame_periods = std::vector<float>();
    generate_pgm_files(video_filename, flags, frame_periods);

    if (on_screen)
    {
        // Framerate gestion
        if (framerate < 0)
        {
            if (frame_periods.size() == 0)
                frame_periods.push_back(1000.0f / 25.0f);
        }
        else
        {
            frame_periods.clear(); 
            frame_periods.push_back(1000.0f / framerate);
        }

        display_all_pgm("pgm", frame_periods.size() == 1, flags, frame_periods);
    }
    else
        convert_all_images("pgm", "ppm");
}

/*
int main() {
  bob_context_t ctx;
  int width = 720;
  int height = 480;
  bob_init(&ctx, width, height);

  // Lire les données du flux vidéo et les envoyer au décodeur
  uint8_t *data;
  int len;
  while (read_video_data(&data, &len)) {
    bob_decode(&ctx, data, len);
  }

  // Allouer de la mémoire pour l'image de sortie
  uint8_t *output = new uint8_t[width * height * 3];

  // Désentrelacer l'image
  bob_deinterlace(&ctx, output);

  // Afficher ou enregistrer l'image désentrelacée

  // Nettoyer et libérer la mémoire
  bob_cleanup(&ctx);
  delete[] output;
  return 0;
}
*/

/*void tmp(int argc, char** argv)
{
    FILE * mpegfile;

    if (argc > 1) {
    mpegfile = fopen (argv[1], "rb");
    if (!mpegfile) {
        fprintf (stderr, "Could not open file \"%s\".\n", argv[1]);
        exit (1);
    }
    } else
    mpegfile = stdin;
}*/