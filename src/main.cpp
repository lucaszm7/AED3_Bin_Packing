#define OLC_PGE_APPLICATION 
#include "include/olcPixelGameEngine.h"
#include "include/utils.hpp"
#include "include/item.hpp"
#include "include/line.hpp"
#include "include/bin.hpp"
#include "include/movingitem.hpp"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

class BinPacking2D : public olc::PixelGameEngine
{
public:
	uint32_t bin_width = 128;
	uint32_t bin_height = 256;
	std::vector<Bin> bins;
	std::vector<Item> items_buffer;
	std::vector<MovingItem> items_buffer_moving_items;
	uint32_t max_Y_value_in_buffer = 0;

	bool toggleHelp = true;

	BinPacking2D()
	{
		sAppName = "BinPacking2D";
	}

public:
	bool Draw(float fElapsedTime)
	{
		Clear(olc::VERY_DARK_BLUE);

		for(auto& bin : bins)
		{
			olc::vd2d binTLbefore = bin.getLeftUpperCorner() - olc::vi2d(1,1);
			olc::vd2d binBRbefore = bin.getRightBottomCorner() + olc::vi2d(1,1);

			olc::vi2d binTLafter;
			olc::vi2d binBRafter;

			WorldToScreen(binTLbefore, binTLafter);
			WorldToScreen(binBRbefore, binBRafter);

			FillRect(binTLafter, binBRafter - binTLafter, olc::BLACK);
			DrawRect(binTLafter, binBRafter - binTLafter, olc::WHITE);

			olc::vi2d binBL(binTLafter.x, binBRafter.y + 10);
			double percentage = 100.0 * ((double)bin.getArea() - (double)bin.getAreaFree()) / (double)bin.getArea();
			std::stringstream percentageStream;
			percentageStream << std::fixed << std::setprecision(2) << percentage;
			std::string binPercentageUsage = "Usage: " + percentageStream.str() + "%";
			DrawString(binBL, binPercentageUsage, olc::YELLOW, 1 * (vScale.x + 0.5));

			olc::vi2d itemTLafter;
			olc::vi2d itemBRafter;
			for(const auto& item : bin.getItemsInBin())
			{
				WorldToScreen(item.getLeftUpperCorner(), itemTLafter);
				WorldToScreen(item.getRightBottomCorner(), itemBRafter);
				FillRect(itemTLafter, itemBRafter - itemTLafter, item.getColor());
			}
		}

		olc::vi2d bufferTLbefore = {0,0};
		olc::vi2d bufferTLafter =  {0,0};
		olc::vi2d bufferBRbefore = {0,0};
		olc::vi2d bufferBRafter =  {0,0};
		if(!items_buffer.empty())
		{
			bufferTLbefore = items_buffer.front().getLeftUpperCorner();
			bufferBRbefore.x = items_buffer.back().getRightBottomCorner().x;
			bufferBRbefore.y = items_buffer.back().getLeftUpperCorner().y + max_Y_value_in_buffer;
			WorldToScreen(bufferTLbefore, bufferTLafter);
			WorldToScreen(bufferBRbefore, bufferBRafter);
			FillRect(bufferTLafter, bufferBRafter - bufferTLafter, olc::BLACK);
			DrawRect(bufferTLafter - olc::vi2d(1,1), (bufferBRafter - bufferTLafter) + olc::vi2d(1,1), olc::WHITE);

			for(const auto& item : items_buffer)
			{
				olc::vi2d itemTLafter;
				olc::vi2d itemBRafter;
				WorldToScreen(item.getLeftUpperCorner(), itemTLafter);
				WorldToScreen(item.getRightBottomCorner(), itemBRafter);
				FillRect(itemTLafter, itemBRafter - itemTLafter, item.getColor());
			}
		}

		if (!items_buffer_moving_items.empty()){
			// All items are in final position?
			bool final_position_items = true;
			
			for (auto& moving_item : items_buffer_moving_items){
				if (moving_item.isInFinalPosition() == false){
					final_position_items = false;
					moving_item.move(fElapsedTime * 1000.0); 

					olc::vi2d itemTLafter;
					olc::vi2d itemBRafter;
					WorldToScreen(moving_item.getItem().getLeftUpperCorner(), itemTLafter);
					WorldToScreen(moving_item.getItem().getRightBottomCorner(), itemBRafter);
					FillRect(itemTLafter, itemBRafter - itemTLafter, moving_item.getItem().getColor());
				}
			}
			if (final_position_items){
				items_buffer_moving_items.clear();
			}
		}
			
		if(vRectDraw)
		{
			olc::vi2d newItemTLafter;
			olc::vi2d newItemBRafter;

			WorldToScreen(vRectStart, newItemTLafter);
			WorldToScreen(vRectNow, newItemBRafter);

			DrawRect(newItemTLafter, newItemBRafter - newItemTLafter, olc::RED);
		}

		return true;
	}

	void OnHandleZoom(float fElapsedTime)
	{
		// Panning and Zoomig, credits to @OneLoneCoder who i'am inpired for
        olc::vd2d vMouse = {(double)GetMouseX(), (double)GetMouseY()};

        // Get the position of the mouse and move the world Final Pos - Inital Pos
        // This make us drag Around the Screen Space, with the OffSet variable
        if(GetMouse(0).bPressed)
        {
            vStartPan = vMouse;
        }

        if(GetMouse(0).bHeld)
        {
            vOffset -= (vMouse - vStartPan) / vScale;
            vStartPan = vMouse;
        }

        olc::vd2d vMouseBeforeZoom;
        ScreenToWorld(vMouse, vMouseBeforeZoom);

		if (GetKey(olc::Key::E).bHeld) 
			vScale *= 1 + (1.1 * fElapsedTime);
		if (GetKey(olc::Key::Q).bHeld) 
			vScale *= 1 - (0.9 * fElapsedTime);
		
		olc::vd2d vMouseAfterZoom;
		ScreenToWorld(vMouse, vMouseAfterZoom);
		vOffset += (vMouseBeforeZoom - vMouseAfterZoom);
	}

	bool OnGui(float fElapsedTime)
	{
        if(toggleHelp)
        {
			olc::Pixel titleColor = olc::GREEN;
			olc::Pixel color = olc::WHITE;
            DrawString(0, 10, "Controles e Ajuda - H:" ,            olc::Pixel(0, 255, 47), 1);
            DrawString(0, 20, "====== Usabilidade ======",          titleColor, 1);
            DrawString(0, 30, "W | A | S | D - Navegacao",          color, 1);
            DrawString(0, 40, "Q | E - Zoom",                       color, 1);
            DrawString(0, 50, "C - Reseta camera",                  color, 1);
            DrawString(0, 60, "Botao Esquerdo - Navegacao",         color, 1);
            DrawString(0, 70, "Botao Direito - Cria item",          color, 1);
            DrawString(0, 90, "====== Atualiza Fila ======",        titleColor, 1);
            DrawString(0, 100,  "J | K | L - 2D: G, M, P",          color, 1);
            DrawString(0, 110, "B | N | M - 1D: G, M, P",           color, 1);
            DrawString(0, 130, "====== Insere Fila ======",         titleColor, 1);
            DrawString(0, 140, "U - Best Fit", color, 1);
            DrawString(0, 150, "I - First Fit", color, 1);
            DrawString(0, 160, "O - Next Fit", color, 1);
        }

        DrawString(ScreenWidth() - 635, ScreenHeight() - 20, "Jelson Rodrigues - Juathan Duarte - Lucas Morais", olc::Pixel(255,255,255,123), 1);
        DrawString(ScreenWidth() - 660, ScreenHeight() - 10, "2D BinPacking - github.com/lucaszm7/AED3_Bin_Packing", olc::Pixel(255,255,255,123), 1);
		return true;
	}

	bool OnHandleControls(float fElapsedTime)
	{
		if (GetKey(olc::Key::H).bReleased)
			toggleHelp = !toggleHelp;

		olc::vd2d vMouseScreen = {(double)GetMouseX(), (double)GetMouseY()};
		olc::vd2d vMouseWorld;
        ScreenToWorld(vMouseScreen, vMouseWorld);

		if(GetMouse(1).bPressed)
        {
			vRectDraw = true;
            vRectStart = vMouseWorld;
			vRectNow = vRectStart;
        }

        if(GetMouse(1).bHeld)
        {
            vRectNow = vMouseWorld;
        }
		
		if (GetKey(olc::Key::A).bHeld)	vOffset -= (olc::vd2d(2.0, 0.0) / vScale) * fElapsedTime * 100;
		if (GetKey(olc::Key::D).bHeld)	vOffset += (olc::vd2d(2.0, 0.0) / vScale) * fElapsedTime * 100;
		if (GetKey(olc::Key::W).bHeld) 	vOffset -= (olc::vd2d(0.0, 2.0) / vScale) * fElapsedTime * 100;
		if (GetKey(olc::Key::S).bHeld)	vOffset += (olc::vd2d(0.0, 2.0) / vScale) * fElapsedTime * 100;

		// Add items to the buffer
		if (GetKey(olc::Key::J).bPressed) {
			addItemToBuffer(Item((rand() % (bin_height / 1)) + 1, (rand() % (bin_width / 1))+ 1));
		}
		if (GetKey(olc::Key::K).bPressed) {
			addItemToBuffer(Item((rand() % (bin_height / 2)) + 1, (rand() % (bin_width / 2))+ 1));
		}
		if (GetKey(olc::Key::L).bPressed) {
			addItemToBuffer(Item((rand() % (bin_height / 4)) + 1, (rand() % (bin_width / 4))+ 1));
		}

		// Add 1d items to the buffer
		if (GetKey(olc::Key::B).bPressed) {
			addItemToBuffer(Item((rand() % (bin_height / 1)) + 1, bin_width ));
		}
		if (GetKey(olc::Key::N).bPressed) {
			addItemToBuffer(Item((rand() % (bin_height / 2)) + 1, bin_width ));
		}
		if (GetKey(olc::Key::M).bPressed) {
			addItemToBuffer(Item((rand() % (bin_height / 4)) + 1, bin_width ));
		}

		// Add all items in the buffer to the bins with best fit strategy
		if (GetKey(olc::Key::U).bPressed) {
			for (auto& item: items_buffer){
				olc::vi2d position_before_insert = item.getLeftUpperCorner();
				olc::vi2d inserted_position;
				if (bestFit(item, inserted_position)){
					std::cout << "Inserted \n";
					item.moveItem(position_before_insert); //reset the item postition
					items_buffer_moving_items.push_back(MovingItem(item, inserted_position, rand() % 1000 + 500));
				}
				else {
					std::cout << "Fail to insert item \n";
				}
			}
			items_buffer.clear();
			max_Y_value_in_buffer = 0;
		}

		// Add all items in the buffer to the bins with first fit strategy
		if (GetKey(olc::Key::I).bPressed) {
			for (auto& item: items_buffer){
				olc::vi2d position_before_insert = item.getLeftUpperCorner();
				olc::vi2d inserted_position;
				if (firstFit(item, inserted_position)){
					std::cout << "Inserted \n";
					item.moveItem(position_before_insert); //reset the item postition
					items_buffer_moving_items.push_back(MovingItem(item, inserted_position, rand() % 1000 + 500));
				}
				else {
					std::cout << "Fail to insert item \n";
				}
			}
			items_buffer.clear();
			max_Y_value_in_buffer = 0;
		}

		// Add all items in the buffer to the bins with next fit strategy
		if (GetKey(olc::Key::O).bPressed) {
			for (auto& item: items_buffer){
				olc::vi2d position_before_insert = item.getLeftUpperCorner();
				olc::vi2d inserted_position;
				if (nextFit(item, inserted_position)){
					std::cout << "Inserted \n";
					item.moveItem(position_before_insert); //reset the item postition
					items_buffer_moving_items.push_back(MovingItem(item, inserted_position, rand() % 1000 + 500));
				}
				else {
					std::cout << "Fail to insert item \n";
				}
			}
			items_buffer.clear();
			max_Y_value_in_buffer = 0;
		}

		// Reset screen position
		if (GetKey(olc::Key::C).bPressed) resetScreenPosition();

		if(GetMouse(1).bReleased)
		{
			vRectDraw = false;
			addItemToBuffer(Item(std::abs(vRectStart.y - vRectNow.y), std::abs(vRectStart.x - vRectNow.x)));
			vRectStart = {0,0};
			vRectNow   = {0,0};
		}

		return true;
	}

	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		int big_items_to_pack = 10;
		int medium_items_to_pack = 20;
		int small_items_to_pack = 15;
		srand(time(NULL));
		
		// Measure the time to pack
		Timer T = Timer();
		olc::vi2d positionInserted;
		for (int c = 0; c < big_items_to_pack; c++){
			std::cout << "Inserted: " << nextFit(Item((rand() % (bin_height)) + 1, (rand() % (bin_width))+ 1), positionInserted) << "\n";
		}
		for (int c = 0; c < medium_items_to_pack; c++){
			std::cout << "Inserted: " << nextFit(Item((rand() % (bin_height / 2)) + 1, (rand() % (bin_width / 2))+ 1), positionInserted) << "\n";
		}
		for (int c = 0; c < small_items_to_pack; c++){
			std::cout << "Inserted: " << nextFit(Item((rand() % (bin_height / 4)) + 1, (rand() % (bin_width / 4))+ 1), positionInserted) << "\n";
		}

		std::cout << "Time Taken: " << T.now() << "s\n";

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		Draw(fElapsedTime);
		OnGui(fElapsedTime);
		OnHandleZoom(fElapsedTime);
		OnHandleControls(fElapsedTime);
		return true;
	}

	bool addItemToBuffer(Item item){
		if (items_buffer.size() == 0) {
			item.moveItem(olc::vi2d(0, -bin_height));
		}
		else {
			item.moveItem(olc::vi2d(items_buffer[items_buffer.size() - 1].getRightBottomCorner().x + 10, -bin_height));	
		}
		items_buffer.push_back(item);
		if (item.getHeight() > max_Y_value_in_buffer) max_Y_value_in_buffer = item.getHeight();
		return true;
	}

	void resetScreenPosition(){
		vOffset = { 0.0, 0.0 };
		vStartPan = { 0.0, 0.0 };
		vScale = { 1.0, 1.0 };
	}

protected:
	bool vRectDraw = false;
	olc::vd2d vRectStart;
	olc::vd2d vRectNow;

protected:
    // Pan & Zoom variables
	olc::vd2d vOffset = { 0.0, 0.0 };
	olc::vd2d vStartPan = { 0.0, 0.0 };
	olc::vd2d vScale = { 1.0, 1.0 };

    void ScreenToWorld(const olc::vi2d& s, olc::vd2d& w)
	{
		w.x = (double)(s.x) / vScale.x + vOffset.x;
		w.y = (double)(s.y) / vScale.y + vOffset.y;
	}

    // Converte coords from Screen Space to World Space
    void WorldToScreen(const olc::vd2d& w, olc::vi2d &s)
	{
		s.x = (int)((w.x - vOffset.x) * vScale.x);
		s.y = (int)((w.y - vOffset.y) * vScale.y);
	}

	// First fit iterates trhought all bins and tries to add in all of them
	bool firstFit(Item item, olc::vi2d &insertedPosition){
		if (item.getHeight() > bin_height || item.getWidth() > bin_width) return false;
		for (int c = 0; c < bins.size(); c++){
			if (bins[c].insert(item)) {
				insertedPosition = bins[c].getItemsInBin()[bins[c].getItemsInBin().size() - 1].getLeftUpperCorner();
				return true;
			}
		}

		createNewBin();
		if (bins[(bins.size() - 1)].insert(item)){
			insertedPosition = bins[(bins.size() - 1)].getItemsInBin()[bins[(bins.size() - 1)].getItemsInBin().size() - 1].getLeftUpperCorner();
			return true;
		} 
		return false;
	}

	// Next fit only considers the last bin
	bool nextFit(Item item, olc::vi2d &insertedPosition){
		if (item.getHeight() > bin_height || item.getWidth() > bin_width) return false;
		if (bins.size() != 0) {
			if (bins[bins.size() - 1].insert(item)) {
				insertedPosition = bins[bins.size() - 1].getItemsInBin()[bins[bins.size() - 1].getItemsInBin().size() - 1].getLeftUpperCorner();
				return true;
			}
		}

		// Creates a new bin and add to the bins vector
		createNewBin();
		if (bins[(bins.size() - 1)].insert(item)){
			insertedPosition = bins[bins.size() - 1].getItemsInBin()[bins[bins.size() - 1].getItemsInBin().size() - 1].getLeftUpperCorner();
			return true;
		}
		return false;
	}

	// Best fit add to the bin that will have the least amount of space unused after the insertion
	bool bestFit(Item item, olc::vi2d &insertedPosition){
		if (item.getHeight() > bin_height || item.getWidth() > bin_width) return false;
		
		// Order the vector of bins by decreasing order of used area
		// Falta ordenar os bins por ordem decrescente de area usada 
		//  std::sort(bins.begin(), bins.end(), compareBinsByAreaUsed);

		return firstFit(item, insertedPosition);
	}

	bool createNewBin() {
		// Create new Bin
		Bin new_bin = Bin(bin_height, bin_width);
		
		new_bin.moveItem(olc::vi2d((bins.size() + 1) * (bin_width + 20), (uint32_t) ScreenHeight() / 2 - bin_height / 2));
		
		bins.push_back(new_bin);
		// olc::vu2d new_bin_coord((bins.size() + 1) * (bin_width + 20), (uint32_t) ScreenHeight() / 2 - bin_height / 2);
		// olc::vu2d new_bin_size(bin_width, bin_height);
		// bins.emplace_back(new_bin_coord, new_bin_size);
		// if (!bins.back().insert(item)) return false;
		return true;
	}

};

int main()
{
	{
		BinPacking2D app;
		if (app.Construct(640, 360, 2, 2)){
			app.Start();
		}
	}
	return 0;
}