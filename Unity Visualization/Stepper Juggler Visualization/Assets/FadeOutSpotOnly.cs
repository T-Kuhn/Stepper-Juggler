using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using DG.Tweening;

public class FadeOutSpotOnly : MonoBehaviour
{
    [SerializeField] private MeshRenderer meshRenderer;

    void Start()
    {
        meshRenderer.material.DOColor(new Color(0f, 0f, 0f, 0f), "_TintColor", 10f);
    }
}
