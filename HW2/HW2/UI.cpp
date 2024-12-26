#include "UI.hpp"


void UI::Render() {
    /* Init */
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(180, 220));

    /* Render */
	ImGui::Begin("Settings");

    ImGui::Checkbox("WaterColor", &option1);
    ImGui::Checkbox("Abstraction", &option2);
    ImGui::Checkbox("Pixelization", &option3);
    ImGui::Checkbox("Sin Wave", &option4);
    ImGui::Checkbox("Magnifier", &option5);
    ImGui::Checkbox("Bloom", &option6);
    ImGui::Checkbox("Normal", &option7);

    if (option1 && lastCheckOption != 1) {
        option2 = option3 = option4 = option5 = option6 = option7 = false;
        lastCheckOption = 1;
    }

    if (option2 && lastCheckOption != 2) {
        option1 = option3 = option4 = option5 = option6 = option7 = false;
        lastCheckOption = 2;
    }

    if (option3 && lastCheckOption != 3) {
        option1 = option2 = option4 = option5 = option6 = option7 = false;
        lastCheckOption = 3;
    }

    if (option4 && lastCheckOption != 4) {
        option1 = option2 = option3 = option5 = option6 = option7 = false;
        lastCheckOption = 4;
    }

    if (option5 && lastCheckOption != 5) {
        option1 = option2 = option3 = option4 = option6 = option7 = false;
        lastCheckOption = 5;
    }

    if (option6 && lastCheckOption != 6) {
        option1 = option2 = option3 = option4 = option5 = option7 = false;
        lastCheckOption = 6;
    }

    if (option7 && lastCheckOption != 7) {
        option1 = option2 = option3 = option4 = option5 = option6 = false;
        lastCheckOption = 7;
    }


    ImGui::SliderInt("Split", &slider, 0, 800);
    
    ImGui::End();

    /* End */
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}