#include <BKAL/Fake/FakeObjectMaker.h>
#include <BKAL/Fake/Edge.h>

#include <stdexcept>
#include <iostream>
#include <memory>
#include <utility>
#include <sstream>

const unsigned int FakeObjectMaker::EDGE=0;
const unsigned int FakeObjectMaker::FACE=1;
unsigned int FakeObjectMaker::EDGE_COUNT;
unsigned int FakeObjectMaker::FACE_COUNT;
const std::map<std::string, int> FakeObjectMaker::BoxFaces = {
    {"front", 0},
    {"back", 1},
    {"top", 2},
    {"bottom", 3},
    {"left", 4},
    {"right", 5}
}; 
const std::map<std::string, int> FakeObjectMaker::CylFaces = {
    {"top", 0},
    {"lateral", 1},
    {"bot", 2}
};
const std::map<std::string, int> FakeObjectMaker::FusTallFaces = {
    {"front", 0},
    {"back", 1},
    {"top", 2},
    {"bottom", 3},
    {"left", 4},
    {"right", 5},
    {"lateral", 6},
    {"cylinder_top", 7},
};

FakeObjectMaker::FakeObjectMaker()
{
}

std::unique_ptr<IEdge> FakeObjectMaker::makeEdge(){
    int name = this->getValue(EDGE);
    return std::unique_ptr<IEdge>(new Fake::Edge(name));
}

std::unique_ptr<IFace> FakeObjectMaker::makeFace(){
    return this->makeFace(this->makeFakeEdges());
}

//Face FakeObjectMaker::makeFace(Edge anEdge){
    //Face aFace = this->makeFace();
    //aFace.myEdges[0] = anEdge;
    //return aFace;
//}

//Face FakeObjectMaker::makeFace(Edge anEdge, int which){
    //Face aFace = this->makeFace();
    //aFace.myEdges[which] = anEdge;
    //return aFace;
//}

std::unique_ptr<IFace> FakeObjectMaker::makeFace(std::vector<Fake::Edge> Edges){
    int name  = this->getValue(FACE);
    Fake::Face* aFace = new Fake::Face(name, Edges);
    return std::unique_ptr<IFace>(aFace);
}

std::unique_ptr<ISolid> FakeObjectMaker::makeBox()
{
    return std::unique_ptr<ISolid>(new Fake::Solid(this->buildBoxFaces()));
}

std::unique_ptr<ISolid> FakeObjectMaker::makeCylinder(){
    Fake::Face topFace = this->makeFakeFace(1);
    Fake::Face latFace = this->makeFakeFace(3);
    Fake::Face botFace = this->makeFakeFace(1);

    latFace.changeEdge(0, topFace.getEdge(0));
    latFace.changeEdge(1, botFace.getEdge(0));

    std::vector<Fake::Face> Faces;
    Faces.push_back(topFace);
    Faces.push_back(latFace);
    Faces.push_back(botFace);
    return std::unique_ptr<ISolid>(new Fake::Solid(Faces));
}

tuple<unique_ptr<ISolid>, vector<pair<FaceIndex, FaceIndex>>> 
    FakeObjectMaker::increaseBoxHeight()
{
    unsigned int frt, bck, top, bot, lft, rgt;
    std::vector<Fake::Face> Faces = this->buildBoxFaces();

    frt = BoxFaces.at("front");
    bck = BoxFaces.at("back");
    top = BoxFaces.at("top");
    bot = BoxFaces.at("bottom");
    lft = BoxFaces.at("left");
    rgt = BoxFaces.at("right");

    // Swap some faces around to challenge the topological namer. This simulates what
    // (usually) happens in opencascade
    Fake::Face temp = Faces[frt];
    Faces[frt] = Faces[bck];
    Faces[bck] = temp;
    temp = Faces[lft];
    Faces[lft] = Faces[rgt];
    Faces[rgt] = temp;

    vector<pair<FaceIndex, FaceIndex>> modifiedFaces = {
        {FaceIndex(frt), FaceIndex(frt)},
        {FaceIndex(bck), FaceIndex(bck)},
        {FaceIndex(lft), FaceIndex(lft)},
        {FaceIndex(rgt), FaceIndex(rgt)},
        {FaceIndex(top), FaceIndex(top)},
        {FaceIndex(bot), FaceIndex(bot)}
    };
    return std::tuple<std::unique_ptr<ISolid>, std::vector<std::pair<FaceIndex, FaceIndex>>>
        (std::unique_ptr<ISolid>(new Fake::Solid(Faces)), modifiedFaces);
}

tuple<unique_ptr<ISolid>, vector<pair<FaceIndex, FaceIndex>>>
    FakeObjectMaker::fuseTallerCylinder()
{
    unsigned int frt, bck, top, bot, lft, rgt, lat, ctp;
    std::vector<Fake::Face> Faces;

    frt = FusTallFaces.at("front");
    bck = FusTallFaces.at("back");
    top = FusTallFaces.at("top");
    bot = FusTallFaces.at("bottom");
    lft = FusTallFaces.at("left");
    rgt = FusTallFaces.at("right");
    lat = FusTallFaces.at("lateral");
    ctp = FusTallFaces.at("cylinder_top");

    for(int i=0; i<=7; i++){
        int numFaces = 4;
        if (i == lat || i == top || i == bot)
        {
            // Since the cylinder will be taller than the box, the lateral face will
            // actually have five edges. Subsequently, the top face of the box will also
            // have five edges.
            //
            // The bottom will have five edges due to the bottom portion of the cylinder,
            // which will be co-planar to the bottom of the box. This edge will be between
            // the lateral face of the cylinder and the (extended) bottom face of the box
            numFaces = 5;
        }
        else if(i == ctp)
        {
            // The circular top of the cylinder only has a single edge.
            numFaces = 1;
        }
        Faces.push_back(this->makeFakeFace(numFaces));
    }

    Faces[top].changeEdge(0, Faces[frt].getEdge(0));
    Faces[bot].changeEdge(0, Faces[frt].getEdge(1));
    Faces[lft].changeEdge(0, Faces[frt].getEdge(2));
    Faces[rgt].changeEdge(0, Faces[frt].getEdge(3));

    Faces[top].changeEdge(1, Faces[bck].getEdge(0));
    Faces[bot].changeEdge(1, Faces[bck].getEdge(1));
    Faces[rgt].changeEdge(1, Faces[bck].getEdge(2));
    Faces[lat].changeEdge(0, Faces[bck].getEdge(3));

    Faces[lft].changeEdge(1, Faces[lat].getEdge(1));
    Faces[lft].changeEdge(2, Faces[top].getEdge(2));
    Faces[lft].changeEdge(3, Faces[bot].getEdge(2));

    Faces[rgt].changeEdge(2, Faces[top].getEdge(3));
    Faces[rgt].changeEdge(3, Faces[bot].getEdge(3));

    Faces[top].changeEdge(4, Faces[lat].getEdge(2));
    Faces[bot].changeEdge(4, Faces[lat].getEdge(3));

    Faces[lat].changeEdge(4, Faces[ctp].getEdge(0));
    
    vector<pair<FaceIndex, FaceIndex>> modifiedFaces = {
        {FaceIndex(frt), FaceIndex(frt)},
        {FaceIndex(lft), FaceIndex(lft)},
        {FaceIndex(rgt), FaceIndex(rgt)},
        {FaceIndex(top), FaceIndex(top)},
        {FaceIndex(bot), FaceIndex(bot)},
        {FaceIndex(bck), FaceIndex(bck)},
        {FaceIndex(), FaceIndex(lat)},
        {FaceIndex(), FaceIndex(ctp)}
    };
    return std::tuple<std::unique_ptr<ISolid>, std::vector<std::pair<FaceIndex, FaceIndex>>>
        (std::unique_ptr<ISolid>(new Fake::Solid(Faces)), modifiedFaces);
}

//std::unique_ptr<ISolid> FakeObjectMaker::filletBox(
        //const std::unique_ptr<ISolid>& aBox,
        //const std::unique_ptr<IEdge>& anEdge)
//{
//}

//FakePartFillet FakeObjectMaker::FilletedBox(){
    //unsigned int frt, top, lft, rgt;

    //// Start with a box
    //FakePartFeature box = this->Box();

    //// Need a new face, since the fillet (or chamfer) creates a face
    //FakeOCCFace filletFace = this->makeFace();

    //// Fillet shares an Edge with four of the box faces.
    //frt = boxFaces.front;
    //top = boxFaces.top;
    //lft = boxFaces.left;
    //rgt = boxFaces.right;

    //unsigned int val1, val2, val3, val4;
    //val1 = this->getValue(FACE);
    //val2 = this->getValue(FACE);
    //val3 = this->getValue(FACE);
    //val4 = this->getValue(FACE);
    //FakeOCCFace Face1(val1), Face2(val2), Face3(val3), Face4(val4);
    //Face1 = box.Shape.Faces[frt];
    //Face2 = box.Shape.Faces[top];
    //Face3 = box.Shape.Faces[lft];
    //Face4 = box.Shape.Faces[rgt];

    //Face1.myEdges[0] = filletFace.myEdges[0];
    //Face2.myEdges[0] = filletFace.myEdges[1];
    //Face3.myEdges.push_back(filletFace.myEdges[2]);
    //Face4.myEdges.push_back(filletFace.myEdges[3]);

    //box.Shape.Faces[frt] = Face1;
    //box.Shape.Faces[top] = Face2;
    //box.Shape.Faces[lft] = Face3;
    //box.Shape.Faces[rgt] = Face4;

    //FakePartFillet fillet(box.Shape, filletFace);
    //return fillet;
//}

//FakePartFeature FakeObjectMaker::Cylinder(){
    //std::vector<FakeOCCEdge> Edges, EdgesTop, EdgesBot;
    //for (int i=1; i<=3; i++){
        //FakeOCCEdge anEdge = this->makeEdge();
        //Edges.push_back(anEdge);
    //}

    //EdgesTop.push_back(Edges[0]);
    //EdgesBot.push_back(Edges[1]);

    //unsigned int val1, val2, val3;
    //val1 = this->getValue(FACE);
    //val2 = this->getValue(FACE);
    //val3 = this->getValue(FACE);

    //FakeOCCFace bot_face(val1, EdgesBot);
    //FakeOCCFace top_face(val2, EdgesTop);
    //FakeOCCFace lat_face(val3, Edges);

    //std::vector<FakeOCCFace> Faces;
    //Faces.push_back(bot_face);
    //Faces.push_back(top_face);
    //Faces.push_back(lat_face);

    //FakePartFeature Fake_feature = FakePartFeature();
    //Fake_feature.Shape.Faces = Faces;
    //return Fake_feature;
//}

//---------------------------------------------------------------------------
//          private methods
//---------------------------------------------------------------------------

int FakeObjectMaker::getValue(unsigned int which) const{
    switch (which) {
        case EDGE:
            EDGE_COUNT++;
            return EDGE_COUNT;
        case FACE:
            FACE_COUNT++;
            return FACE_COUNT;
        default:
            throw std::invalid_argument("which must be EDGE or FACE");
    }
}

Fake::Face FakeObjectMaker::makeFakeFace(unsigned int num_edges){
    int name = this->getValue(FACE);
    Fake::Face outFace(name, this->makeFakeEdges(num_edges));
    return outFace;
}

Fake::Edge FakeObjectMaker::makeFakeEdge(){
    int name = this->getValue(EDGE);
    return Fake::Edge(name);
}

std::vector<Fake::Edge> FakeObjectMaker::makeFakeEdges(unsigned int count){
    std::vector<Fake::Edge> outEdges;
    for (int i=0; i<count ; i++)
    {
        int name = this->getValue(EDGE);
        outEdges.push_back(Fake::Edge(name));
    }
    return outEdges;
}

// ------------------- Private Methods ------------------------
std::vector<Fake::Face> FakeObjectMaker::buildBoxFaces()
{
    unsigned int frt, bck, top, bot, lft, rgt;
    std::vector<Fake::Face> Faces;

    frt = BoxFaces.at("front");
    bck = BoxFaces.at("back");
    top = BoxFaces.at("top");
    bot = BoxFaces.at("bottom");
    lft = BoxFaces.at("left");
    rgt = BoxFaces.at("right");

    for(int i=1; i<=6; i++){
        Fake::Face aFace = this->makeFakeFace();
        Faces.push_back(aFace);
    }

    Faces[top].changeEdge(0, Faces[frt].getEdge(0));
    Faces[bot].changeEdge(0, Faces[frt].getEdge(1));
    Faces[lft].changeEdge(0, Faces[frt].getEdge(2));
    Faces[rgt].changeEdge(0, Faces[frt].getEdge(3));

    Faces[top].changeEdge(1, Faces[bck].getEdge(0));
    Faces[bot].changeEdge(1, Faces[bck].getEdge(1));
    Faces[lft].changeEdge(1, Faces[bck].getEdge(2));
    Faces[rgt].changeEdge(1, Faces[bck].getEdge(3));

    Faces[lft].changeEdge(2, Faces[top].getEdge(2));
    Faces[lft].changeEdge(3, Faces[bot].getEdge(2));

    Faces[rgt].changeEdge(2, Faces[top].getEdge(3));
    Faces[rgt].changeEdge(3, Faces[bot].getEdge(3));
    return Faces;
}
