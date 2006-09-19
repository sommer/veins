import
    "BaseMobility",
    "BaseUtility";

module BaseObject
    submodules:
        utility: BaseUtility;
            display: "p=130,38,rect;b=24,24;o=black";
        mobility: BaseMobility;
            display: "p=130,172;i=cogwheel2";

    display: "p=10,10;b=180,200,rect;o=white";
endmodule

